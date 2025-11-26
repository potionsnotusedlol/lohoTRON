#include "GameProcess.h"
#include <QtMath>
#include <GL/gl.h>

GameProcess::GameProcess(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_fieldSize(100)
    , m_gridSize(100)
    , m_cellSize(2.0f)
    , m_mapHalfSize(0.0f)
    , m_keyForward(false)
    , m_keyBackward(false)
    , m_keyLeft(false)
    , m_keyRight(false)
    , m_maxForwardSpeed(14.0f)
    , m_maxBackwardSpeed(8.0f)
    , m_accel(18.0f)
    , m_brake(35.0f)
    , m_friction(10.0f)
    , m_turnSpeed(2.0f)
    , m_maxLean(qDegreesToRadians(35.0f))
    , m_leanSpeed(8.0f)
    , m_camYaw(0.0f)
    , m_camPitch(-0.6f)
    , m_camDistance(14.0f)
    , m_camDistanceCur(14.0f)
    , m_camSmooth(8.0f)
    , m_camTargetHeight(2.0f)
    , m_mouseSensitivity(0.0020f)
    , m_mouseCaptured(false)
    , m_trailTTL(5.0f)
    , m_trailMinDist(0.3f)
    , m_trailSize(0.8f)
    , m_trailHeight(1.2f)
    , m_lastTimeMs(0)
    , m_time(0.0f)
    , m_tickTimer(nullptr)
    , m_paused(false)
{
    setFocusPolicy(Qt::StrongFocus);

    m_mapHalfSize = 0.5f * m_cellSize * static_cast<float>(m_gridSize);

    m_bike.pos = QVector3D(0.0f, 0.0f, 0.0f);
    m_bike.yaw = 0.0f;
    m_bike.speed = 0.0f;
    m_bike.lean = 0.0f;

    m_camTarget = m_bike.pos + QVector3D(0.0f, m_camTargetHeight, 0.0f);

    m_timer.start();
    m_lastTimeMs = m_timer.elapsed();
    m_time = 0.0f;

    m_tickTimer = new QTimer(this);
    connect(m_tickTimer, &QTimer::timeout, this, &GameProcess::onTick);
    m_tickTimer->start(16);
}

GameProcess::~GameProcess()
{
}

void GameProcess::setFieldSize(int n)
{
    if (n < 2) n = 2;
    m_fieldSize = n;
    m_gridSize = n;
    m_mapHalfSize = 0.5f * m_cellSize * static_cast<float>(m_gridSize);

    m_bike.pos = QVector3D(0.0f, 0.0f, 0.0f);
    m_bike.yaw = 0.0f;
    m_bike.speed = 0.0f;
    m_bike.lean = 0.0f;

    m_trail.clear();
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
    qint64 now = m_timer.elapsed();
    float dt = static_cast<float>(now - m_lastTimeMs) / 1000.0f;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.1f) dt = 0.1f;
    m_lastTimeMs = now;
    m_time += dt;

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
    p.setRenderHint(QPainter::Antialiasing);
    if (m_paused) {
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("PAUSE"));
    }
}

void GameProcess::keyPressEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat()) {
        QOpenGLWidget::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
        m_keyForward = true;
        break;
    case Qt::Key_S:
    case Qt::Key_Down:
        m_keyBackward = true;
        break;
    case Qt::Key_A:
    case Qt::Key_Left:
        m_keyLeft = true;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_keyRight = true;
        break;
    case Qt::Key_Space:
        m_paused = !m_paused;
        break;
    default:
        break;
    }

    QOpenGLWidget::keyPressEvent(event);
}

void GameProcess::keyReleaseEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat()) {
        QOpenGLWidget::keyReleaseEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
        m_keyForward = false;
        break;
    case Qt::Key_S:
    case Qt::Key_Down:
        m_keyBackward = false;
        break;
    case Qt::Key_A:
    case Qt::Key_Left:
        m_keyLeft = false;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_keyRight = false;
        break;
    default:
        break;
    }

    QOpenGLWidget::keyReleaseEvent(event);
}

void GameProcess::mousePressEvent(QMouseEvent* event)
{
    m_mouseCaptured = true;
    m_lastMousePos = event->pos();
    QOpenGLWidget::mousePressEvent(event);
}

void GameProcess::mouseReleaseEvent(QMouseEvent* event)
{
    QOpenGLWidget::mouseReleaseEvent(event);
}

void GameProcess::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_mouseCaptured) {
        m_lastMousePos = event->pos();
        m_mouseCaptured = true;
    } else {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();

        m_camYaw   -= static_cast<float>(delta.x()) * m_mouseSensitivity;
        m_camPitch -= static_cast<float>(delta.y()) * m_mouseSensitivity;

        float minPitch = -1.2f;
        float maxPitch = 0.3f;
        if (m_camPitch < minPitch) m_camPitch = minPitch;
        if (m_camPitch > maxPitch) m_camPitch = maxPitch;
    }

    QOpenGLWidget::mouseMoveEvent(event);
}



void GameProcess::onTick()
{
    update();
}


void GameProcess::updateSimulation(float dt)
{
    if (dt <= 0.0f) return;

    static QVector3D vel(0.0f, 0.0f, 0.0f);

    QVector3D dir(0.0f, 0.0f, 0.0f);
    if (m_keyForward)
        dir.setZ(dir.z() - 1.0f);
    if (m_keyBackward)
        dir.setZ(dir.z() + 1.0f);
    if (m_keyLeft)
        dir.setX(dir.x() - 1.0f);
    if (m_keyRight)
        dir.setX(dir.x() + 1.0f);

    if (!dir.isNull())
        dir.normalize();

    QVector3D acc(0.0f, 0.0f, 0.0f);
    if (!dir.isNull())
        acc += dir * m_accel;

    vel += acc * dt;

    float speed = vel.length();
    float maxSpeed = m_maxForwardSpeed;
    if (speed > maxSpeed && speed > 0.0f)
        vel *= (maxSpeed / speed);

    if (dir.isNull() && speed > 0.0f) {
        float decel = m_friction;
        float dec = decel * dt;
        if (dec >= speed)
            vel = QVector3D(0.0f, 0.0f, 0.0f);
        else
            vel *= (1.0f - dec / speed);
        speed = vel.length();
    }

    QVector3D newPos = m_bike.pos + vel * dt;

    float border = m_mapHalfSize - m_cellSize * 2.0f;
    newPos.setX(clampf(newPos.x(), -border, border));
    newPos.setZ(clampf(newPos.z(), -border, border));
    m_bike.pos = newPos;

    if (speed > 0.001f) {
        float desiredYaw = std::atan2(vel.x(), -vel.z());
        float diff = wrapAngle(desiredYaw - m_bike.yaw);
        float maxTurn = 3.0f * dt;
        if (diff >  maxTurn) diff =  maxTurn;
        if (diff < -maxTurn) diff = -maxTurn;
        m_bike.yaw = wrapAngle(m_bike.yaw + diff);
    }

    updateCamera(dt);
}



void GameProcess::updateCamera(float dt)
{
    if (dt <= 0.0f) return;

    float t = 1.0f - std::exp(-m_camSmooth * dt);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    QVector3D desiredTarget = m_bike.pos + QVector3D(0.0f, m_camTargetHeight, 0.0f);
    m_camTarget += (desiredTarget - m_camTarget) * t;

    m_camDistanceCur += (m_camDistance - m_camDistanceCur) * t;
}



void GameProcess::updateTrail(float dt)
{
    Q_UNUSED(dt);

    QVector3D basePos = m_bike.pos;
    if (m_trail.empty() || (basePos - m_trail.back().pos).length() >= m_trailMinDist) {
        TrailPoint tp;
        tp.pos = basePos;
        tp.t = m_time;
        m_trail.push_back(tp);
    }

    while (!m_trail.empty() && (m_time - m_trail.front().t) > m_trailTTL) {
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
    float yaw = m_bike.yaw + m_camYaw;

    float cp = std::cos(m_camPitch);
    float sp = std::sin(m_camPitch);
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);

    QVector3D forward(sy * cp, sp, -cy * cp);

    QVector3D eye = m_camTarget - forward.normalized() * m_camDistanceCur;
    QVector3D up(0.0f, 1.0f, 0.0f);

    QMatrix4x4 view;
    view.setToIdentity();
    view.lookAt(eye, m_camTarget, up);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view.constData());
}


void GameProcess::drawScene3D()
{
    drawGround();
    drawGrid();
    drawTrail();
    drawBike();
}

void GameProcess::drawGround()
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
}

void GameProcess::drawGrid()
{
    float half = m_mapHalfSize;

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

    float S = 1.6f;
    float L = S;
    float W = S;
    float H = S;

    float x0 = -W * 0.5f;
    float x1 = +W * 0.5f;
    float y0 = 0.0f;
    float y1 = H;
    float z0 = -L * 0.5f;
    float z1 = +L * 0.5f;

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

    const float cubeHeight = 1.6f;
    const float baseOffset = 0.05f;
    const float columnHeight = cubeHeight * 0.75f;

    for (const TrailPoint& tp : m_trail) {
        float age = m_time - tp.t;
        if (age < 0.0f || age > m_trailTTL)
            continue;

        float alpha = clampf(1.0f - age / m_trailTTL, 0.0f, 1.0f);

        QVector3D base = tp.pos;
        float halfSize = m_trailSize;

        float x0 = base.x() - halfSize;
        float x1 = base.x() + halfSize;
        float z0 = base.z() - halfSize;
        float z1 = base.z() + halfSize;
        float y0 = base.y() - baseOffset;
        float y1 = y0 + columnHeight;

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

        glEnd();
    }

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

float GameProcess::clampf(float v, float a, float b)
{
    if (v < a) v = a;
    if (v > b) v = b;
    return v;
}

float GameProcess::lerpf(float a, float b, float t)
{
    return a + (b - a) * t;
}

float GameProcess::wrapAngle(float a)
{
    const float twoPi = 2.0f * static_cast<float>(M_PI);
    while (a <= -static_cast<float>(M_PI)) a += twoPi;
    while (a >  static_cast<float>(M_PI)) a -= twoPi;
    return a;
}
