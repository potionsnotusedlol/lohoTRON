#include "GameProcess.h"

GameProcess::GameProcess(QWidget* parent) : QOpenGLWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    m_root.reset();
    m_scene_manager = nullptr;
    setMouseTracking(true);  
    setCursor(Qt::BlankCursor);  
    m_render_window = nullptr;
    m_fieldSize = 100;
    m_gridSize = m_fieldSize;
    m_cellSize = 2.0f;
    m_mapHalfSize = 0.5f * m_cellSize * static_cast<float>(m_gridSize);
    m_paused = false;
    m_camYaw = 0.0f;
    m_camPitch = -0.4f;
    m_camDistance = 12.0f;
    m_camDistanceCur = m_camDistance;
    m_camTargetHeight = 3.0f;
    m_camSmooth = 8.0f;
    m_camTarget = QVector3D(0.0f, m_camTargetHeight, 0.0f);
    m_rmbDown = false;
    m_mouseCaptured = false;
    m_mouseSensitivity = 0.0018f;
    m_keyForward = false;
    m_keyBackward = false;
    m_keyLeft = false;
    m_keyRight = false;
    m_maxForwardSpeed = 30.0f;
    m_maxBackwardSpeed = 15.0f;
    m_acceleration = 40.0f;
    m_brakeDecel = 60.0f;
    m_friction = 18.0f;
    m_turnSpeed = 2.8f;
    m_maxLeanAngle = Ogre::Degree(38.0f).valueRadians();
    m_leanSpeed = 7.0f;
    m_trailTTL = 12.0f;
    m_trailMinDist = 0.35f;
    m_trailColumnSize = 0.8f;
    m_trailColumnHeight = 3.0f;
    m_time = 0.0f;
    m_lastTimeMs = 0;
    m_bike.pos = QVector3D(0.0f, 0.0f, 0.0f);
    m_bike.yaw = 0.0f;
    m_bike.speed = 0.0f;
    m_bike.lean = 0.0f;
    m_bike.color = QVector3D(0.0f, 0.8f, 1.0f);
    m_bike.human = true;
    m_bike.alive = true;
    m_bike.prevPos = m_bike.pos;
    m_bike.currPos = m_bike.pos;
    m_bike.aiTurnTimer = 0.0f;
    m_bike.aiTurnDir = 0.0f;
    m_bikes.clear();
    m_bikeTrails.clear();
    pauseWindow = new GamePauseWindow(this);

    connect(pauseWindow, &GamePauseWindow::resumeGame, this, [this]() {
        m_paused = false;
    });

    connect(pauseWindow, &GamePauseWindow::cancelPause, this, [this]() {
    m_paused = false;
});

    connect(pauseWindow, &GamePauseWindow::restartGame, this, [this]() {
        resetGame();
        m_paused = false;
    });

    connect(pauseWindow, &GamePauseWindow::exitToMenu, this, [this]() {
        m_paused = false;
        emit exitToMainMenu();
    });

    int botCount = 4;
    int total = 1 + botCount;

    m_bikes.resize(total);
    m_bikeTrails.resize(total);
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    QVector3D colors[5] = {
        QVector3D(0.0f, 0.8f, 1.0f),
        QVector3D(1.0f, 0.3f, 0.3f),
        QVector3D(0.3f, 1.0f, 0.3f),
        QVector3D(1.0f, 0.8f, 0.2f),
        QVector3D(0.8f, 0.3f, 1.0f)
    };
    float spawnRadius = m_mapHalfSize * 0.6f;

    for (int i = 0; i < total; ++i) {
        Bike& b = m_bikes[i];
        float angle = (static_cast<float>(i) / static_cast<float>(total)) * 2.0f * static_cast<float>(M_PI);
        float x = std::cos(angle) * spawnRadius * 0.3f, z = std::sin(angle) * spawnRadius * 0.3f;

        b.pos   = QVector3D(x, 0.0f, z);
        b.yaw   = -angle + static_cast<float>(M_PI);
        b.speed = 0.0f;
        b.lean  = 0.0f;
        b.color = colors[i % 5];
        b.human = (i == 0);
        b.alive = true;
        b.prevPos = b.pos;
        b.currPos = b.pos;
        b.aiTurnTimer = 0.5f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        b.aiTurnDir   = 0.0f;
        m_bikeTrails[i].clear();
        TrailPoint tp;
        tp.pos = b.pos;
        tp.time = m_time;
        m_bikeTrails[i].push_back(tp);
    }

    if (!m_bikes.empty()) m_camTarget = m_bikes[0].pos + QVector3D(0.0f, m_camTargetHeight, 0.0f);

    m_timer.start();
    m_lastTimeMs = m_timer.elapsed();
    m_tickTimer = new QTimer(this);
    connect(m_tickTimer, SIGNAL(timeout()), this, SLOT(onTick()));
    m_tickTimer->start(16);
}

GameProcess::~GameProcess() {}

void GameProcess::setFieldSize(int n) {
    m_fieldSize = std::max(10, n);
    m_gridSize = m_fieldSize;
    m_mapHalfSize = 0.5f * m_cellSize * static_cast<float>(m_gridSize);
}

void GameProcess::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.03f, 1.0f);
}

void GameProcess::killBike(int idx)
{
    if (idx < 0 || idx >= static_cast<int>(m_bikes.size())) return;
    Bike& b = m_bikes[idx];
    if (!b.alive) return;

    b.alive = false;
    ++m_deadCount;

    if (b.human) {
        int total = static_cast<int>(m_bikes.size());
        m_playerRank = total - m_deadCount + 1;
    }
}

void GameProcess::resizeGL(int w, int h) { glViewport(0, 0, w, h); }

void GameProcess::paintGL() {
    if (!m_timer.isValid()) {
        m_timer.start();
        m_lastTimeMs = m_timer.elapsed();
    }

    qint64 now = m_timer.elapsed();
    float dt = static_cast<float>(now - m_lastTimeMs) * 0.001f;
    m_lastTimeMs = now;

    if (dt > 0.05f) dt = 0.05f;
    if (dt < 0.0f) dt = 0.0f;

    if (!m_paused) {
        m_time += dt;
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
    p.setPen(QPen(Qt::white));
    if (m_roundOver) {
        QFont f = p.font();
        f.setPointSize(36);
        f.setBold(true);
        p.setFont(f);
        p.setPen(QPen(Qt::red));
        p.drawText(rect(), Qt::AlignCenter, m_roundText);
    }


}

void GameProcess::keyPressEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat()) {
        QOpenGLWidget::keyPressEvent(event);

        return;
    }

    if (m_roundOver) {
        m_roundOver = false;
        resetGame();                  
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
        break;
    case Qt::Key_Escape:
    if (pauseWindow->isVisible()) {
        pauseWindow->closeByEsc();
    } else {
        m_paused = true;
        pauseWindow->exec();
    }
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

void GameProcess::mousePressEvent(QMouseEvent* event) {
    m_mouseCaptured = true;
    m_lastMousePos = event->pos();

    QOpenGLWidget::mousePressEvent(event);
}

void GameProcess::mouseReleaseEvent(QMouseEvent* event) { QOpenGLWidget::mouseReleaseEvent(event); }

void GameProcess::mouseMoveEvent(QMouseEvent* event)
{
    static bool first = true;
    static QPoint lastPos;

    if (first) {
        first = false;
        lastPos = event->pos();
        QOpenGLWidget::mouseMoveEvent(event);
        return;
    }

    QPoint cur  = event->pos();
    QPoint delta = cur - lastPos;
    lastPos = cur;

    float dx = static_cast<float>(delta.x());
    float dy = static_cast<float>(delta.y());

    float sens = m_mouseSensitivity;

    m_camYaw   -= dx * sens;
    m_camPitch -= dy * sens;

    float minPitch = -static_cast<float>(M_PI) * 0.5f + 0.1f;
    float maxPitch =  static_cast<float>(M_PI) * 0.5f - 0.1f;
    if (m_camPitch < minPitch) m_camPitch = minPitch;
    if (m_camPitch > maxPitch) m_camPitch = maxPitch;

    QOpenGLWidget::mouseMoveEvent(event);
}

void GameProcess::onTick() { update(); }

void GameProcess::updateSimulation(float dt)
{
    if (dt <= 0.0f) return;

    m_time += dt;

    float bikeRadius  = 0.8f;
    float trailRadius = 0.3f;

    for (int i = 0; i < static_cast<int>(m_bikes.size()); ++i) {
        Bike& b = m_bikes[i];
        if (!b.alive) continue;

        b.prevPos = b.pos;

        float turnInput   = 0.0f;
        bool  moveForward = false;

        if (b.human) {
            if (m_keyLeft)  turnInput += 1.0f;
            if (m_keyRight) turnInput -= 1.0f;
            moveForward = true;
        } else {
            moveForward = true;
        
            const float lookAheadDist  = 200.0f;   
            const float avoidThreshold = 2.0f;   
            const float attackDist2    = 400.0f; 
            const float minDotAttack   = 0.1f;   
        
            const Bike& player = m_bikes[0];
        
            QVector3D localForward(0.0f, 0.0f, -1.0f);
            QMatrix4x4 rot;
            rot.setToIdentity();
            rot.rotate(b.yaw * 180.0f / static_cast<float>(M_PI), 0.0f, 1.0f, 0.0f);
            QVector3D forwardDir = rot.map(localForward).normalized();
            QVector3D rightDir(forwardDir.z(), 0.0f, -forwardDir.x());
        
            bool needAvoid = false;
            float avoidTurn = 0.0f;
        
            QVector3D ahead = b.pos + forwardDir * lookAheadDist;
        
            int nBots = static_cast<int>(m_bikes.size());
            float safeR = 0.8f + 0.4f;
        
            for (int owner = 0; owner < nBots && !needAvoid; ++owner) {
                const std::vector<TrailPoint>& trail = m_bikeTrails[owner];
                if (trail.empty()) continue;
            
                for (size_t k = 0; k < trail.size(); ++k) {
                    const TrailPoint& tp = trail[k];
                    QVector3D p = tp.pos;
                    p.setY(0.0f);
                
                    QVector3D v = p - b.pos;
                    v.setY(0.0f);
                
                    float proj = QVector3D::dotProduct(v, forwardDir);
                    if (proj < 0.0f || proj > lookAheadDist) continue;
                
                    QVector3D projPoint = b.pos + forwardDir * proj;
                    projPoint.setY(0.0f);
                
                    QVector3D diff = p - projPoint;
                    diff.setY(0.0f);
                    float dist2 = diff.lengthSquared();
                
                    if (dist2 <= avoidThreshold * avoidThreshold) {
                        float side = QVector3D::dotProduct(p - b.pos, rightDir);
                        avoidTurn = (side >= 0.0f) ? -1.0f : 1.0f;
                        needAvoid = true;
                        break;
                    }
                }
            }
        
            if (needAvoid) {
                turnInput = avoidTurn;
            } else {
                QVector3D toPlayer = player.pos - b.pos;
                toPlayer.setY(0.0f);
                float distPlayer2 = toPlayer.lengthSquared();
            
                if (distPlayer2 > 0.0001f) toPlayer.normalize();
            
                float dotForward = QVector3D::dotProduct(forwardDir, toPlayer);
            
                if (distPlayer2 <= attackDist2 && dotForward > minDotAttack) {
                    float side = QVector3D::dotProduct(toPlayer, rightDir.normalized());
                    if (side > 0.0f)
                        turnInput = -1.0f;
                    else
                        turnInput = 1.0f;
                
                    turnInput *= 0.7f + 0.6f * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
                } else {
                    b.aiTurnTimer -= dt;
                    if (b.aiTurnTimer <= 0.0f) {
                        b.aiTurnTimer = 0.5f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 1.5f;
                        float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                        if (r < 0.3f)      b.aiTurnDir = -1.0f;
                        else if (r > 0.7f) b.aiTurnDir =  1.0f;
                        else               b.aiTurnDir =  0.0f;
                    }
                    turnInput = b.aiTurnDir;
                }
            }
        }


        float turnSpeed = m_turnSpeed;
        float currentTurnSpeed = 0.0f;
        if (turnInput > 0.0f) currentTurnSpeed =  turnSpeed;
        if (turnInput < 0.0f) currentTurnSpeed = -turnSpeed;

        b.yaw = wrapPi(b.yaw + currentTurnSpeed * dt);

        QVector3D localForward(0.0f, 0.0f, -1.0f);
        QMatrix4x4 rot;
        rot.setToIdentity();
        rot.rotate(b.yaw * 180.0f / static_cast<float>(M_PI), 0.0f, 1.0f, 0.0f);
        QVector3D dir = rot * localForward;

        float maxSpeed    = m_maxForwardSpeed;
        float accelFactor = m_acceleration;
        float decelFactor = m_friction;

        if (moveForward) {
            float desiredSpeed = maxSpeed;
            float dv = (desiredSpeed - b.speed) * accelFactor * dt;
            b.speed += dv;
        } else {
            float dv = (-b.speed) * decelFactor * dt;
            b.speed += dv;
        }

        if (b.speed > maxSpeed) b.speed = maxSpeed;
        if (b.speed < 0.0f)     b.speed = 0.0f;

        QVector3D vel    = dir.normalized() * b.speed;
        QVector3D newPos = b.pos + vel * dt;

        float border = m_mapHalfSize - m_cellSize * 2.0f;
        if (newPos.x() >  border) newPos.setX( border);
        if (newPos.x() < -border) newPos.setX(-border);
        if (newPos.z() >  border) newPos.setZ( border);
        if (newPos.z() < -border) newPos.setZ(-border);

        b.pos     = newPos;
        b.currPos = b.pos;

        std::vector<TrailPoint>& trail = m_bikeTrails[i];
        if (trail.empty() || (b.pos - trail.back().pos).length() >= m_trailMinDist) {
            TrailPoint tp;
            tp.pos  = b.pos;
            tp.time = m_time;
            trail.push_back(tp);
        }
    }

    int n = static_cast<int>(m_bikes.size());

    for (int i = 0; i < n; ++i) {
        if (!m_bikes[i].alive) continue;
        for (int j = i + 1; j < n; ++j) {
            if (!m_bikes[j].alive) continue;

            QVector3D d = m_bikes[i].pos - m_bikes[j].pos;
            d.setY(0.0f);
            float r = bikeRadius * 2.0f;
            if (d.lengthSquared() <= r * r) {
                killBike(i);
                killBike(j);
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        if (!m_bikes[i].alive) continue;
        Bike& A = m_bikes[i];

        float hitR  = bikeRadius + trailRadius;
        float hitR2 = hitR * hitR;

        for (int owner = 0; owner < n; ++owner) {
            const std::vector<TrailPoint>& trail = m_bikeTrails[owner];
            if (trail.empty()) continue;

            for (const TrailPoint& tp : trail) {
                if (owner == i && (m_time - tp.time) < 0.1f) continue;

                QVector3D d = A.pos - tp.pos;
                d.setY(0.0f);
                if (d.lengthSquared() <= hitR2) {
                    killBike(i);
                    break;
                }
            }
            if (!A.alive) break;
        }
    }

    if (!m_roundOver) {
        int aliveCount = 0;
        for (int i = 0; i < n; ++i)
            if (m_bikes[i].alive) ++aliveCount;

        if (aliveCount <= 1) {
            m_roundOver = true;

            int total = n;
            if (m_bikes[0].alive) {
                m_playerRank = 1;
            } else if (m_playerRank == 0) {
                m_playerRank = total - m_deadCount + 1;
            }

            m_roundText = QString("РАУНД ЗАКОНЧЕН\nВы заняли %1 место").arg(m_playerRank);
        }
    }

    if (!m_roundOver) {
        updateCamera(dt);
    }
}

void GameProcess::updateCamera(float dt)
{
    if (m_bikes.empty()) return;
    const Bike& player = m_bikes[0];

    QVector3D desiredTarget = player.pos + QVector3D(0.0f, m_camTargetHeight, 0.0f);

    float t = 1.0f - std::exp(-m_camSmooth * dt);
    t = clampf(t, 0.0f, 1.0f);
    m_camTarget += (desiredTarget - m_camTarget) * t;

    m_camDistanceCur += (m_camDistance - m_camDistanceCur) * t;

    m_camYaw = -player.yaw;
    m_camPitch = -0.4f;
}

void GameProcess::updateTrail(float dt) {
    if (m_trailTTL <= 0.0f) return;

    for (size_t i = 0; i < m_bikeTrails.size(); ++i) {
        std::vector<TrailPoint>& trail = m_bikeTrails[i];

        while (!trail.empty() && (m_time - trail.front().time) > m_trailTTL) trail.erase(trail.begin());
    }

    Q_UNUSED(dt);
}

void GameProcess::setupProjection() {
    int w = width(), h = height();

    if (h == 0) h = 1;

    float aspect = static_cast<float>(w) / static_cast<float>(h);
    QMatrix4x4 proj;
    proj.setToIdentity();
    proj.perspective(60.0f, aspect, 0.1f, 2000.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj.constData());
}

void GameProcess::setupView() {
    if (m_bikes.empty()) return;

    const Bike& player = m_bikes[0];

    float yaw = player.yaw;

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




void GameProcess::drawScene3D() {
    drawGroundGrid();
    drawTrail();
    drawBike();
}

void GameProcess::drawGroundGrid() {
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

void GameProcess::showEvent(QShowEvent* event)
{
    QOpenGLWidget::showEvent(event);
    resetGame();
}


void GameProcess::resetGame()
{
    m_time   = 0.0f;
    m_paused = false;

    m_bike.pos  = QVector3D(0.0f, 0.0f, 0.0f);
    m_bike.yaw  = 0.0f;
    m_bike.speed = 0.0f;
    m_bike.lean  = 0.0f;
    m_bike.color = QVector3D(0.0f, 0.8f, 1.0f);
    m_bike.human = true;
    m_bike.alive = true;
    m_bike.prevPos = m_bike.pos;
    m_bike.currPos = m_bike.pos;
    m_bike.aiTurnTimer = 0.0f;
    m_bike.aiTurnDir   = 0.0f;

    m_bikes.clear();
    m_bikeTrails.clear();

    int botCount = 4;      
    int total    = 1 + botCount;

    m_bikes.resize(total);
    m_bikeTrails.resize(total);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    QVector3D colors[5] = {
        QVector3D(0.0f, 0.8f, 1.0f),
        QVector3D(1.0f, 0.3f, 0.3f),
        QVector3D(0.3f, 1.0f, 0.3f),
        QVector3D(1.0f, 0.8f, 0.2f),
        QVector3D(0.8f, 0.3f, 1.0f)
    };

    float spawnRadius = m_mapHalfSize * 0.6f;

    for (int i = 0; i < total; ++i) {
        Bike& b = m_bikes[i];

        float angle = (static_cast<float>(i) / static_cast<float>(total))
                    * 2.0f * static_cast<float>(M_PI);
        float x = std::cos(angle) * spawnRadius * 0.3f;
        float z = std::sin(angle) * spawnRadius * 0.3f;

        b.pos   = QVector3D(x, 0.0f, z);
        b.yaw   = -angle + static_cast<float>(M_PI);
        b.speed = 0.0f;
        b.lean  = 0.0f;
        b.color = colors[i % 5];
        b.human = (i == 0);
        b.alive = true;
        b.prevPos = b.pos;
        b.currPos = b.pos;
        b.aiTurnTimer = 0.5f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        b.aiTurnDir   = 0.0f;

        m_bikeTrails[i].clear();
        TrailPoint tp;
        tp.pos  = b.pos;
        tp.time = m_time;
        m_bikeTrails[i].push_back(tp);
    }

    if (!m_bikes.empty()) {
        m_camTarget = m_bikes[0].pos + QVector3D(0.0f, m_camTargetHeight, 0.0f);
    }

    if (m_timer.isValid())
        m_lastTimeMs = m_timer.elapsed();
    else {
        m_timer.start();
        m_lastTimeMs = m_timer.elapsed();
    }
}

void GameProcess::drawBike()
{
    float rad2deg = 180.0f / static_cast<float>(M_PI);
    glDisable(GL_BLEND);

    for (size_t i = 0; i < m_bikes.size(); ++i) {
        const Bike& b = m_bikes[i];
        if (!b.alive) continue;

        glPushMatrix();

        glTranslatef(b.pos.x(), b.pos.y(), b.pos.z());

        glRotatef(b.yaw * rad2deg, 0.0f, 1.0f, 0.0f);
        glRotatef(b.lean * rad2deg, 0.0f, 0.0f, 1.0f);

        float L = 2.4f;
        float W = 1.2f;
        float H = 1.6f;

        float x0 = -W * 0.5f, x1 = +W * 0.5f;
        float y0 = 0.0f,      y1 = H;
        float z0 = -L * 0.5f, z1 = +L * 0.5f;

        glBegin(GL_QUADS);

        glColor3f(b.color.x(), b.color.y(), b.color.z());
        glVertex3f(x0, y0, z1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x0, y1, z1);

        glColor3f(b.color.x() * 0.5f, b.color.y() * 0.5f, b.color.z() * 0.5f);
        glVertex3f(x1, y0, z0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);

        glColor3f(b.color.x() * 0.7f, b.color.y() * 0.7f, b.color.z() * 0.7f);
        glVertex3f(x0, y0, z1); glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0); glVertex3f(x0, y1, z1);

        glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1); glVertex3f(x1, y1, z0);

        glColor3f(b.color.x(), b.color.y(), b.color.z());
        glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);

        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex3f(x0, y0, z1); glVertex3f(x1, y0, z1);
        glVertex3f(x1, y0, z0); glVertex3f(x0, y0, z0);

        glEnd();

        glPopMatrix();
    }
}


void GameProcess::drawTrail()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_CULL_FACE);

    for (size_t i = 0; i < m_bikes.size(); ++i) {
        const Bike& b = m_bikes[i];
        const std::vector<TrailPoint>& trail = m_bikeTrails[i];
        if (trail.size() < 2) continue;

        QVector3D col = b.color;
        float baseR = col.x();
        float baseG = col.y();
        float baseB = col.z();

        float halfWidth = 0.35f;
        float height    = 3.0f;
        float baseY     = 0.0f;

        for (size_t k = 0; k + 1 < trail.size(); ++k) {
            const TrailPoint& a = trail[k];
            const TrailPoint& c = trail[k + 1];

            float ageA = m_time - a.time;
            float ageC = m_time - c.time;
            if (ageA < 0.0f || ageA > m_trailTTL) continue;
            if (ageC < 0.0f || ageC > m_trailTTL) continue;

            float alphaA = 1.0f - ageA / m_trailTTL;
            float alphaC = 1.0f - ageC / m_trailTTL;
            if (alphaA < 0.2f) alphaA = 0.2f;
            if (alphaC < 0.2f) alphaC = 0.2f;

            QVector3D p0 = a.pos;
            QVector3D p1 = c.pos;
            p0.setY(baseY);
            p1.setY(baseY);

            QVector3D dir = (p1 - p0);
            dir.setY(0.0f);
            if (dir.lengthSquared() < 0.0001f) continue;
            dir.normalize();
            QVector3D perp(-dir.z(), 0.0f, dir.x());

            QVector3D b1 = p0 - perp * halfWidth;
            QVector3D b2 = p0 + perp * halfWidth;
            QVector3D b3 = p1 + perp * halfWidth;
            QVector3D b4 = p1 - perp * halfWidth;

            QVector3D t1 = b1 + QVector3D(0.0f, height, 0.0f);
            QVector3D t2 = b2 + QVector3D(0.0f, height, 0.0f);
            QVector3D t3 = b3 + QVector3D(0.0f, height, 0.0f);
            QVector3D t4 = b4 + QVector3D(0.0f, height, 0.0f);

            glBegin(GL_QUADS);

            glColor4f(baseR, baseG, baseB, alphaA * 0.9f);
            glVertex3f(b1.x(), b1.y(), b1.z());
            glVertex3f(b2.x(), b2.y(), b2.z());
            glColor4f(baseR, baseG, baseB, alphaC * 0.9f);
            glVertex3f(b3.x(), b3.y(), b3.z());
            glVertex3f(b4.x(), b4.y(), b4.z());

            glColor4f(baseR, baseG, baseB, alphaA * 0.7f);
            glVertex3f(t2.x(), t2.y(), t2.z());
            glVertex3f(t1.x(), t1.y(), t1.z());
            glColor4f(baseR, baseG, baseB, alphaC * 0.7f);
            glVertex3f(t4.x(), t4.y(), t4.z());
            glVertex3f(t3.x(), t3.y(), t3.z());

            glColor4f(baseR, baseG, baseB, alphaC * 0.8f);
            glVertex3f(t1.x(), t1.y(), t1.z());
            glVertex3f(t2.x(), t2.y(), t2.z());
            glVertex3f(t3.x(), t3.y(), t3.z());
            glVertex3f(t4.x(), t4.y(), t4.z());

            glColor4f(baseR * 0.6f, baseG * 0.6f, baseB * 0.6f, alphaA * 0.5f);
            glVertex3f(b1.x(), b1.y(), b1.z());
            glVertex3f(b4.x(), b4.y(), b4.z());
            glVertex3f(b3.x(), b3.y(), b3.z());
            glVertex3f(b2.x(), b2.y(), b2.z());

            glColor4f(baseR, baseG, baseB, alphaA * 0.8f);
            glVertex3f(b1.x(), b1.y(), b1.z());
            glVertex3f(t1.x(), t1.y(), t1.z());
            glVertex3f(t2.x(), t2.y(), t2.z());
            glVertex3f(b2.x(), b2.y(), b2.z());

            glColor4f(baseR, baseG, baseB, alphaC * 0.8f);
            glVertex3f(b4.x(), b4.y(), b4.z());
            glVertex3f(t4.x(), t4.y(), t4.z());
            glVertex3f(t3.x(), t3.y(), t3.z());
            glVertex3f(b3.x(), b3.y(), b3.z());

            glEnd();
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

float GameProcess::clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }

float GameProcess::lerpf(float a, float b, float t) { return a + (b - a) * t; }

float GameProcess::wrapPi(float a) {
    const float twoPi = 2.0f * static_cast<float>(M_PI);

    while (a <= -static_cast<float>(M_PI)) a += twoPi;

    while (a >   static_cast<float>(M_PI)) a -= twoPi;

    return a;
}