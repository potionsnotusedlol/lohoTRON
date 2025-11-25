#include "GameProcess.h"

#include <QtMath>

GameProcess::GameProcess(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    m_gridSize    = m_fieldSize;
    m_mapHalfSize = 0.5f * m_cellSize * static_cast<float>(m_gridSize);

    m_bike.pos   = QVector3D(0.0f, 0.0f, 0.0f);
    m_bike.yaw   = 0.0f;
    m_bike.speed = 0.0f;
    m_bike.lean  = 0.0f;

    m_camYaw         = 0.0f;
    m_camPitch       = -0.6f;
    m_camDistance    = 18.0f;
    m_camDistanceCur = m_camDistance;
    m_camTargetHeight = 3.0f;

    m_timer.start();
    m_lastTimeMs = m_timer.elapsed();
    m_time       = 0.0f;

    m_tickTimer = new QTimer(this);
    connect(m_tickTimer, &QTimer::timeout, this, &GameProcess::onTick);
    m_tickTimer->start(16);
}

GameProcess::~GameProcess() = default;

void GameProcess::setFieldSize(int n)
{
    if (n < 2) n = 2;
    m_fieldSize = n;
    m_gridSize  = n;
    m_mapHalfSize = 0.5f * m_cellSize * static_cast<float>(m_gridSize);

    m_bike.pos = QVector3D(0.0f, 0.0f, 0.0f);
    m_trail.clear();

    update();
}

void GameProcess::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.0f, 0.0f, 0.03f, 1.0f);
}

void GameProcess::resizeGL(int w, int h)
{
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

void GameProcess::paintGL()
{
    if (!m_timer.isValid())
        m_timer.start();
    qint64 now = m_timer.elapsed();
    float dt = static_cast<float>(now - m_lastTimeMs) / 1000.0f;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.1f) dt = 0.1f;
    m_lastTimeMs = now;
    m_time      += dt;

    if (!m_paused) {
        updateSimulation(dt);
        updateTrail(dt);
    } else {
        updateCamera(dt);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setupProjection();
    setupView();
    drawScene3D();

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    if (m_paused) {
        QPen pen(Qt::white);
        pen.setWidth(2);
        p.setPen(pen);
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("PAUSED"));
    }
}

void GameProcess::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_paused = !m_paused;
        update();
        return;
    }

    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
        m_keyForward = true; break;
    case Qt::Key_S:
    case Qt::Key_Down:
        m_keyBackward = true; break;
    case Qt::Key_A:
    case Qt::Key_Left:
        m_keyLeft = true; break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_keyRight = true; break;
    default:
        break;
    }

    QOpenGLWidget::keyPressEvent(event);
}

void GameProcess::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
        m_keyForward = false; break;
    case Qt::Key_S:
    case Qt::Key_Down:
        m_keyBackward = false; break;
    case Qt::Key_A:
    case Qt::Key_Left:
        m_keyLeft = false; break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_keyRight = false; break;
    default:
        break;
    }

    QOpenGLWidget::keyReleaseEvent(event);
}

void GameProcess::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        m_rmbDown = true;
        m_lastMousePos = event->pos();
    }
    QOpenGLWidget::mousePressEvent(event);
}

void GameProcess::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        m_rmbDown = false;
    }
    QOpenGLWidget::mouseReleaseEvent(event);
}

void GameProcess::mouseMoveEvent(QMouseEvent* event)
{
    if (m_rmbDown) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();

        m_camYaw   -= delta.x() * m_mouseSensitivity;
        m_camPitch -= delta.y() * m_mouseSensitivity;

        float minPitch = -1.3f;
        float maxPitch =  0.3f;
        if (m_camPitch < minPitch) m_camPitch = minPitch;
        if (m_camPitch > maxPitch) m_camPitch = maxPitch;

        update();
    }
    QOpenGLWidget::mouseMoveEvent(event);
}

void GameProcess::onTick()
{
    update();
}

void GameProcess::updateSimulation(float dt)
{
    float acc = 0.0f;
    if (m_keyForward)
        acc += m_acceleration;
    if (m_keyBackward)
        acc += (m_bike.speed > 0.0f ? -m_brakeDecel : -m_acceleration);

    if (m_keyForward || m_keyBackward) {
        m_bike.speed += acc * dt;
    } else {
        if (m_bike.speed > 0.0f) {
            m_bike.speed -= m_friction * dt;
            if (m_bike.speed < 0.0f) m_bike.speed = 0.0f;
        } else if (m_bike.speed < 0.0f) {
            m_bike.speed += m_friction * dt;
            if (m_bike.speed > 0.0f) m_bike.speed = 0.0f;
        }
    }

    m_bike.speed = clampf(m_bike.speed, -m_maxBackwardSpeed, m_maxForwardSpeed);

    float turn = (m_keyLeft ? 1.0f : 0.0f) + (m_keyRight ? -1.0f : 0.0f);

    float speedNorm = std::min(1.0f, std::fabs(m_bike.speed) / m_maxForwardSpeed);
    float turnFactor = 0.3f + 0.7f * speedNorm; 

    float dirSign = (m_bike.speed >= 0.0f ? 1.0f : -1.0f);
    m_bike.yaw += turn * m_turnSpeed * turnFactor * dt * dirSign;


    float speedFactor = std::min(1.0f, std::fabs(m_bike.speed) / m_maxForwardSpeed);
    float targetLean = turn * m_maxLeanAngle * speedFactor;
    float tLean = std::min(1.0f, m_leanSpeed * dt);
    m_bike.lean = lerpf(m_bike.lean, targetLean, tLean);

    float cy = std::cos(m_bike.yaw);
    float sy = std::sin(m_bike.yaw);
    QVector3D forward(sy, 0.0f, -cy);

    QVector3D cand = m_bike.pos + forward * (m_bike.speed * dt);

    float border = m_mapHalfSize - m_cellSize * 2.0f; 
    if (cand.x() > border)  cand.setX(border);
    if (cand.x() < -border) cand.setX(-border);
    if (cand.z() > border)  cand.setZ(border);
    if (cand.z() < -border) cand.setZ(-border);

    m_bike.pos = cand;

    updateCamera(dt);
}

void GameProcess::updateCamera(float dt)
{
    m_camTarget = m_bike.pos + QVector3D(0.0f, m_camTargetHeight, 0.0f);

    if (!m_rmbDown) {
        float diff = wrapPi(m_bike.yaw - m_camYaw);

        const float followSpeed = 1.5f;

        float t = std::min(1.0f, followSpeed * dt);

        m_camYaw += diff * t;
    }

    float tz = 1.0f - std::exp(-m_camSmooth * dt);
    m_camDistanceCur += (m_camDistance - m_camDistanceCur) * tz;
}


void GameProcess::updateTrail(float /*dt*/)
{
    QVector3D pos = m_bike.pos + QVector3D(0.0f, 0.0f, 0.0f);

    if (m_trail.empty() ||
        (pos - m_trail.back().pos).length() >= m_trailMinDist)
    {
        m_trail.push_back({pos, m_time});
    }

    while (!m_trail.empty() && (m_time - m_trail.front().time) > m_trailTTL) {
        m_trail.erase(m_trail.begin());
    }
}

void GameProcess::setupProjection()
{
    int w = width();
    int h = height();
    if (h == 0) h = 1;

    float aspect = static_cast<float>(w) / static_cast<float>(h);

    QMatrix4x4 proj;
    proj.setToIdentity();
    proj.perspective(60.0f, aspect, 0.1f, 2000.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj.constData());
}

void GameProcess::setupView()
{
    float cy = std::cos(m_camYaw);
    float sy = std::sin(m_camYaw);
    float cp = std::cos(m_camPitch);
    float sp = std::sin(m_camPitch);

    QVector3D offset(
        m_camDistanceCur * sy * cp,     
        -m_camDistanceCur * sp,         
        m_camDistanceCur * cy * cp      
    );

    QVector3D eye    = m_camTarget + offset;
    QVector3D center = m_camTarget;
    QVector3D up(0.0f, 1.0f, 0.0f);

    QMatrix4x4 view;
    view.setToIdentity();
    view.lookAt(eye, center, up);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view.constData());
}

void GameProcess::drawScene3D()
{
    drawGroundGrid();
    drawTrail();
    drawBike();
}

void GameProcess::drawGroundGrid()
{
    float half = m_mapHalfSize;

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor3f(0.02f, 0.02f, 0.06f);
    glVertex3f(-half, -0.5f, -half);
    glVertex3f(+half, -0.5f, -half);
    glVertex3f(+half, -0.5f, +half);
    glVertex3f(-half, -0.5f, +half);
    glEnd();

    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.6f, 1.0f);

    for (int i = 0; i <= m_gridSize; ++i) {
        float p = (static_cast<float>(i) * m_cellSize) - half;

        glVertex3f(-half, -0.49f, p);
        glVertex3f(+half, -0.49f, p);

        glVertex3f(p, -0.49f, -half);
        glVertex3f(p, -0.49f, +half);
    }

    glEnd();
}

void GameProcess::drawBike()
{
    glPushMatrix();

    glTranslatef(m_bike.pos.x(), m_bike.pos.y(), m_bike.pos.z());

    glRotatef(qRadiansToDegrees(m_bike.yaw), 0.0f, 1.0f, 0.0f);
    glRotatef(qRadiansToDegrees(m_bike.lean), 0.0f, 0.0f, 1.0f);

    float S = 1.6f;   
    float L = S, W = S, H = S;

    float x0 = -W * 0.5f, x1 = +W * 0.5f;
    float y0 = 0.0f,      y1 = H;
    float z0 = -L * 0.5f, z1 = +L * 0.5f;

    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.8f, 1.0f);

    glVertex3f(x0, y0, z1);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z1);

    glVertex3f(x1, y0, z0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x1, y1, z0);

    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y0, z1);
    glVertex3f(x0, y1, z1);
    glVertex3f(x0, y1, z0);

    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z1);

    glVertex3f(x0, y1, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z0);
    glVertex3f(x0, y1, z0);

    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1);
    glVertex3f(x0, y0, z1);

    glEnd();

    glPopMatrix();
}

void GameProcess::drawTrail()
{
    if (m_trail.empty())
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
    glDisable(GL_CULL_FACE);          

    for (const TrailPoint& tp : m_trail) {
        float age = m_time - tp.time;
        if (age < 0.0f || age > m_trailTTL) continue;

        float alpha = clampf(1.0f - age / m_trailTTL, 0.0f, 1.0f);

        QVector3D base = tp.pos;
        float halfSize = m_trailColumnSize;
        float h        = m_trailColumnHeight;

        float x0 = base.x() - halfSize;
        float x1 = base.x() + halfSize;
        float z0 = base.z() - halfSize;
        float z1 = base.z() + halfSize;
        float y0 = base.y();
        float y1 = base.y() + h;

        glBegin(GL_QUADS);
        glColor4f(0.0f, 0.9f, 1.2f, alpha);

        glVertex3f(x0, y0, z1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x0, y1, z1);

        glVertex3f(x1, y0, z0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);

        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y1, z0);

        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1);

        glVertex3f(x0, y1, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z0);
        glVertex3f(x0, y1, z0);

        glEnd();
    }

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

float GameProcess::clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float GameProcess::lerpf(float a, float b, float t)
{
    return a + (b - a) * t;
}

float GameProcess::wrapPi(float a)
{
    const float twoPi = 2.0f * static_cast<float>(M_PI);
    while (a <= -static_cast<float>(M_PI)) a += twoPi;
    while (a >   static_cast<float>(M_PI)) a -= twoPi;
    return a;
}
