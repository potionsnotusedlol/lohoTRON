#include "GameProcess.h"

GameProcess::GameProcess(QWidget* parent) : QOpenGLWidget(parent) {
    gameOverWindow = new GameOverWindow(this);

    connect(this, &GameProcess::matchOver, this, [this](bool win, int killedBots, int wonRounds){
    gameOverWindow->setMatchResult(win, killedBots, wonRounds);
    gameOverWindow->show();
    });
    connect(gameOverWindow, &GameOverWindow::restartGame, this, &GameProcess::resetGameSlot);
    connect(gameOverWindow, &GameOverWindow::exitToMenu, this, &GameProcess::exitToMenuInternal);    

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
    m_trailColumnHeight= 3.0f;
    m_time = 0.0f;
    m_lastTimeMs = 0;
    m_roundOver = false;
    m_playerRank = 0;
    m_deadCount = 0;

    if (m_botCount < 1) m_botCount = 3;

    if (m_roundsCount < 1) m_roundsCount = 3;

    m_currentRound = 1;
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

    connect(pauseWindow, &GamePauseWindow::resumeGame, this, [this]() { m_paused = false; });
    connect(pauseWindow, &GamePauseWindow::cancelPause, this, [this]() { m_paused = false; });
    connect(pauseWindow, &GamePauseWindow::restartGame, this,
        [this]() {
            resetGameSlot();
            m_paused = false;
        }
    );
    connect(pauseWindow, &GamePauseWindow::exitToMenu, this,
        [this]() {
            m_paused = false;
            emit exitToMainMenu();
        }
    );

    if (m_botCount < 1) m_botCount = 1;

    int total = 1 + m_botCount;     

    m_bikes.resize(total);
    m_bikeTrails.resize(total);
    m_totalBots = m_botCount;
    m_aliveBots = m_botCount;
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    QVector3D colors[6] = {
        QVector3D(0.243f, 0.337f, 0.133f), // olive leaf
        QVector3D(0.141f, 0.431f, 0.725f), // bright marine
        QVector3D(0.306f, 0.008f, 0.314f), // dark amethyst
        QVector3D(0.918f, 0.604f, 0.698f), // pink mist
        QVector3D(0.22f, 0.302f, 0.282f), // dark slate grey
        QVector3D(0.8f, 0.247f, 0.047f) // red ochre
    };

    float spawnRadius = m_mapHalfSize * 0.75f;
    Bike& player_bike = m_bikes[0];
    float player_baseAngle = 0, jitter = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 0.4f;
    float player_angle = player_baseAngle + jitter, player_radiusJitter = 0.15f * spawnRadius;
    float r = spawnRadius - player_radiusJitter + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * player_radiusJitter, x = std::cos(player_angle) * r, z = std::sin(player_angle) * r;

    player_bike.pos = QVector3D(0, 0.0f, z);
    player_bike.yaw = + static_cast<float>(M_PI) - player_angle;
    player_bike.speed = 0.0f;
    player_bike.lean = 0.0f;
    player_bike.color = colors[getColor()];
    player_bike.human = true;
    player_bike.alive = true;
    player_bike.prevPos = player_bike.pos;
    player_bike.currPos = player_bike.pos;
    player_bike.aiTurnTimer = 0.5f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    player_bike.aiTurnDir = 0.0f;
    m_bikeTrails[0].clear();

    TrailPoint player_tp;
    player_tp.pos = player_bike.pos;
    player_tp.time = m_time;

    m_bikeTrails[0].push_back(player_tp);

    for (int i = 1; i < total; ++i) {
        Bike& b = m_bikes[i];
        float baseAngle = (static_cast<float>(i) / static_cast<float>(total)) * 2.0f * static_cast<float>(M_PI), jitter = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 0.4f;
        float angle = baseAngle + jitter, radiusJitter = 0.15f * spawnRadius;
        float r = spawnRadius - radiusJitter + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * radiusJitter, x = std::cos(angle) * r, z = std::sin(angle) * r;
        
        b.pos = QVector3D(x, 0.0f, z);
        b.yaw = -angle + static_cast<float>(M_PI); 
        b.speed = 0.0f;
        b.lean = 0.0f;
        b.color = colors[5];
        b.human = (i == 0);
        b.alive = true;
        b.prevPos = b.pos;
        b.currPos = b.pos;
        b.aiTurnTimer = 0.5f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        b.aiTurnDir = 0.0f;
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

void GameProcess::setFieldSize(int n) {
    m_fieldSize = std::max(10, n);
    m_gridSize = m_fieldSize;
    m_mapHalfSize = 0.5f * m_cellSize * static_cast<float>(m_gridSize);
}

void GameProcess::setBotCount(int n) { m_botCount = std::max(1, n); }

void GameProcess::setRoundsCount(int n) {
    m_roundsCount = std::max(1, n);
    m_currentRound = 1;
}

void GameProcess::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.03f, 1.0f);
}

void GameProcess::killBike(int idx) {
    if (idx < 0 || idx >= static_cast<int>(m_bikes.size())) return;

    Bike& b = m_bikes[idx];

    if (!b.alive) return;

    b.alive = false;
    ++m_deadCount;

    if (!b.human && m_aliveBots > 0) --m_aliveBots;

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
        // m_time += dt;
        updateSimulation(dt);
        updateTrail(dt);
    } else updateCamera(dt);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setupProjection();
    setupView();
    drawScene3D();

    const int margin = 12, hud_height = 48;
    QRect hud_rect(
        margin,
        height() - hud_height - margin,
        width() - margin * 2,
        hud_height
    );
    QPainter p(this);
    QString roundStr = QString("ROUND %1 / %2").arg(m_currentRound).arg(m_roundsCount), botsStr = QString("ENEMIES: %1 / %2").arg(m_aliveBots).arg(m_totalBots);
    QFont f("Wattauchimma");
    f.setPointSize(72);
    p.setFont(f);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(QColor(0, 191, 255));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(hud_rect, 8, 8);
    p.setPen(QColor(0, 191, 255));
    p.drawText(
        hud_rect.adjusted(10, 0, -10, 0),
        Qt::AlignLeft | Qt::AlignVCenter,
        roundStr
    );
    p.drawText(
        hud_rect.adjusted(10, 0, -10, 0),
        Qt::AlignRight | Qt::AlignVCenter,
        botsStr
    );

    if (m_roundOver) {
        QFont f2 = p.font();
        f2.setPointSize(36);
        f2.setBold(true);

        p.setFont(f2);
        p.setPen(QPen(Qt::red));
        p.drawText(rect(), Qt::AlignCenter, m_roundText);
    }
}

void GameProcess::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        QOpenGLWidget::keyPressEvent(event);

        return;
    }

    if (m_roundOver && !m_matchOver) {
    m_roundOver = false;
    resetGame(false);
    QOpenGLWidget::keyPressEvent(event);
    return;
}

    QJsonObject root = loadConfigRoot();
    QJsonObject player = root.value("player").toObject();
    QJsonArray keys_binded = player.value("key_bindings").toArray();
    Qt::Key key_forward = QKeySequence::fromString(keys_binded[0].toString(), QKeySequence::PortableText)[0].key(), key_backward = QKeySequence::fromString(keys_binded[1].toString(), QKeySequence::PortableText)[0].key(), key_left = QKeySequence::fromString(keys_binded[2].toString(), QKeySequence::PortableText)[0].key(), key_right = QKeySequence::fromString(keys_binded[3].toString(), QKeySequence::PortableText)[0].key();
    
    if (event->key() == key_forward || event->key() == Qt::Key_Up) m_keyForward = true;
    else if (event->key() == key_backward || event->key() == Qt::Key_Down) m_keyBackward = true;
    else if (event->key() == key_left || event->key() == Qt::Key_Left) m_keyLeft = true;
    else if (event->key() == key_right || event->key() == Qt::Key_Right) m_keyRight = true;
    else if (event->key() == Qt::Key_Escape) {
        if (pauseWindow->isVisible()) pauseWindow->reject();
        else {
            m_paused = true;
            pauseWindow->exec();
        }
    }

    QOpenGLWidget::keyPressEvent(event);
}

void GameProcess::exitToMenuInternal() {
    m_matchOver = false;
    m_paused = false;
    m_roundOver = false;
    m_deadCount = 0;
    m_playerRank = 0;

    if (m_tickTimer)
        m_tickTimer->stop();

    emit exitToMainMenu();
}


void GameProcess::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        QOpenGLWidget::keyReleaseEvent(event);

        return;
    }

    QJsonObject root = loadConfigRoot();
    QJsonObject player = root.value("player").toObject();
    QJsonArray keys_binded = player.value("key_bindings").toArray();
    Qt::Key key_forward = QKeySequence::fromString(keys_binded[0].toString(), QKeySequence::PortableText)[0].key(), key_backward = QKeySequence::fromString(keys_binded[1].toString(), QKeySequence::PortableText)[0].key(), key_left = QKeySequence::fromString(keys_binded[2].toString(), QKeySequence::PortableText)[0].key(), key_right = QKeySequence::fromString(keys_binded[3].toString(), QKeySequence::PortableText)[0].key();

    if (event->key() == key_forward || event->key() == Qt::Key_Up) m_keyForward = false;
    else if (event->key() == key_backward|| event->key() == Qt::Key_Down) m_keyBackward = false;
    else if (event->key() == key_left || event->key() == Qt::Key_Left) m_keyLeft = false;
    else if (event->key() == key_right || event->key() == Qt::Key_Right) m_keyRight = false;

    QOpenGLWidget::keyReleaseEvent(event);
}

void GameProcess::mousePressEvent(QMouseEvent* event) {
    m_mouseCaptured = true;
    m_lastMousePos = event->pos();
    QOpenGLWidget::mousePressEvent(event);
}

void GameProcess::mouseReleaseEvent(QMouseEvent* event) { QOpenGLWidget::mouseReleaseEvent(event); }

void GameProcess::mouseMoveEvent(QMouseEvent* event) {
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

    float dx = static_cast<float>(delta.x()), dy = static_cast<float>(delta.y());
    float sens = m_mouseSensitivity;

    m_camYaw -= dx * sens;
    m_camPitch -= dy * sens;

    float minPitch = -static_cast<float>(M_PI) * 0.5f + 0.1f, maxPitch = static_cast<float>(M_PI) * 0.5f - 0.1f;

    if (m_camPitch < minPitch) m_camPitch = minPitch;

    if (m_camPitch > maxPitch) m_camPitch = maxPitch;

    QOpenGLWidget::mouseMoveEvent(event);
}

void GameProcess::onTick() { update(); }

void GameProcess::updateSimulation(float dt) {
    if (dt <= 0.0f) return;

    m_time += dt;
    float bikeRadius = 0.8f, trailRadius = 0.3f;
    int n = static_cast<int>(m_bikes.size());

    if (!m_roundOver) {
    int aliveCount = 0;
    for (int i = 0; i < n; ++i) if (m_bikes[i].alive) ++aliveCount;

    if (aliveCount <= 1) {
    m_roundOver = true;

    bool playerAlive = m_bikes[0].alive;
    if (playerAlive) ++m_roundsWon;
    else {
        ++m_roundsLost;
        ++m_botsCrashedIntoPlayer;
    }

    if (m_currentRound >= m_roundsCount) {
        m_matchOver = true;
        m_paused = true;
        gameOverWindow->setMatchResult(m_roundsWon > m_roundsLost, m_botsCrashedIntoPlayer, m_roundsWon);
        gameOverWindow->show();
    } else {
        ++m_currentRound; 
        m_roundOver = true;
        m_paused = true;
        m_roundText = "ROUND OVER\nPress any key";
    }
}
}

    if (m_paused || m_matchOver)
        return;

    for (int i = 0; i < n; ++i) {
        Bike& b = m_bikes[i];
        if (!b.alive) continue;

        b.prevPos = b.pos;

        float turnInput = 0.0f;
        bool moveForward = false;

        if (b.human) {
            if (m_keyLeft)  turnInput += 1.0f;
            if (m_keyRight) turnInput -= 1.0f;
            moveForward = true;
        } else {
            moveForward = true;
            const float lookAheadDist = 200.0f, avoidThreshold = 2.0f, attackDist2 = 400.0f, minDotAttack = 0.1f;
            const Bike& player = m_bikes[0];

            QVector3D localForward(0,0,-1);
            QMatrix4x4 rot;
            rot.setToIdentity();
            rot.rotate(b.yaw * 180.0f / static_cast<float>(M_PI), 0,1,0);
            QVector3D forwardDir = rot.map(localForward).normalized();
            QVector3D rightDir(forwardDir.z(),0,-forwardDir.x());

            bool needAvoid = false;
            float avoidTurn = 0.0f;

            for (int owner = 0; owner < n && !needAvoid; ++owner) {
                const auto& trail = m_bikeTrails[owner];
                if (trail.empty()) continue;

                for (const auto& tp : trail) {
                    QVector3D p = tp.pos; p.setY(0);
                    QVector3D v = p - b.pos; v.setY(0);
                    float proj = QVector3D::dotProduct(v, forwardDir);
                    if (proj < 0 || proj > lookAheadDist) continue;

                    QVector3D projPoint = b.pos + forwardDir * proj;
                    projPoint.setY(0);
                    QVector3D diff = p - projPoint;
                    diff.setY(0);

                    if (diff.lengthSquared() <= avoidThreshold * avoidThreshold) {
                        float side = QVector3D::dotProduct(p - b.pos, rightDir);
                        avoidTurn = (side >= 0.0f) ? -1.0f : 1.0f;
                        needAvoid = true;
                        break;
                    }
                }
            }

            if (needAvoid) turnInput = avoidTurn;
            else {
                QVector3D toPlayer = player.pos - b.pos;
                toPlayer.setY(0);
                float dist2 = toPlayer.lengthSquared();
                if (dist2 > 0.0001f) toPlayer.normalize();
                float dotForward = QVector3D::dotProduct(forwardDir, toPlayer);

                if (dist2 <= attackDist2 && dotForward > minDotAttack) {
                    float side = QVector3D::dotProduct(toPlayer, rightDir.normalized());
                    turnInput = (side > 0) ? -1.0f : 1.0f;
                    turnInput *= 0.4f + 0.4f * (static_cast<float>(std::rand()) / RAND_MAX);
                } else {
                    b.aiTurnTimer -= dt;
                    if (b.aiTurnTimer <= 0.0f) {
                        b.aiTurnTimer = 0.5f + static_cast<float>(std::rand()) / RAND_MAX * 1.5f;
                        float r = static_cast<float>(std::rand()) / RAND_MAX;
                        if (r < 0.3f) b.aiTurnDir = -1.0f;
                        else if (r > 0.7f) b.aiTurnDir = 1.0f;
                        else b.aiTurnDir = 0.0f;
                    }
                    turnInput = b.aiTurnDir;
                }
            }
        }

        float currentTurnSpeed = (turnInput > 0) ? m_turnSpeed : (turnInput < 0) ? -m_turnSpeed : 0.0f;
        b.yaw = wrapPi(b.yaw + currentTurnSpeed * dt);

        QMatrix4x4 rot2;
        rot2.setToIdentity();
        rot2.rotate(b.yaw * 180.0f / static_cast<float>(M_PI), 0,1,0);
        QVector3D dir = rot2 * QVector3D(0,0,-1);

        float maxSpeed = m_maxForwardSpeed;
        float accelFactor = m_acceleration, decelFactor = m_friction;
        if (moveForward) b.speed += (maxSpeed - b.speed) * accelFactor * dt;
        else b.speed += (-b.speed) * decelFactor * dt;

        b.speed = qBound(0.0f, b.speed, maxSpeed);

        QVector3D newPos = b.pos + dir.normalized() * b.speed * dt;
        float border = m_mapHalfSize - m_cellSize * 2.0f;
        newPos.setX(qBound(-border, newPos.x(), border));
        newPos.setZ(qBound(-border, newPos.z(), border));
        b.pos = b.currPos = newPos;

        auto& trail = m_bikeTrails[i];
        if (trail.empty() || (b.pos - trail.back().pos).length() >= m_trailMinDist) {
            trail.push_back({b.pos, m_time});
        }
    }

    for (int i = 0; i < n; ++i) {
        if (!m_bikes[i].alive) continue;
        for (int j = i+1; j < n; ++j) {
            if (!m_bikes[j].alive) continue;
            if ((m_bikes[i].pos - m_bikes[j].pos).lengthSquared() <= bikeRadius*2*bikeRadius*2) {
                killBike(i);
                killBike(j);
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        if (!m_bikes[i].alive) continue;
        Bike& A = m_bikes[i];
        float hitR2 = (bikeRadius + trailRadius) * (bikeRadius + trailRadius);

        for (int owner = 0; owner < n; ++owner) {
            const auto& trail = m_bikeTrails[owner];
            if (trail.empty()) continue;
            for (const auto& tp : trail) {
                if (owner == i && (m_time - tp.time) < 0.1f) continue;
                if ((A.pos - tp.pos).lengthSquared() <= hitR2) {
                    killBike(i);
                    break;
                }
            }
            if (!A.alive) break;
        }
    }

    if (!m_roundOver)
        updateCamera(dt);
}



void GameProcess::updateCamera(float dt) {
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
    float cp = std::cos(m_camPitch), sp = std::sin(m_camPitch), cy = std::cos(m_camYaw), sy = std::sin(m_camYaw);
    QVector3D forward(sy * cp, sp, -cy * cp);
    QVector3D eye = m_camTarget - forward.normalized() * m_camDistanceCur, up(0.0f, 1.0f, 0.0f);
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

void GameProcess::showEvent(QShowEvent* event) {
    QOpenGLWidget::showEvent(event);
    resetGameSlot();
}

void GameProcess::resetGameSlot() {
    resetGame(true);
}


void GameProcess::resetGame(bool newMatch) {
    m_matchOver = false;
    m_paused = false;
    m_roundOver = false;
    m_deadCount = 0;
    m_playerRank = 0;

    if (newMatch) {
        m_currentRound = 1;
        m_roundsWon = 0;
        m_roundsLost = 0;
        m_botsCrashedIntoPlayer = 0;
    }

    if (m_botCount < 1) m_botCount = 1;

    int total = 1 + m_botCount;   

    m_bikes.clear();
    m_bikeTrails.clear();
    m_bikes.resize(total);
    m_bikeTrails.resize(total);
    m_totalBots = m_botCount;
    m_aliveBots = m_botCount;
    m_time = 0.0f;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    if (m_bikes.empty()) return;

    QVector3D colors[6] = {
        QVector3D(0.243f, 0.337f, 0.133f),
        QVector3D(0.141f, 0.431f, 0.725f),
        QVector3D(0.306f, 0.008f, 0.314f),
        QVector3D(0.918f, 0.604f, 0.698f),
        QVector3D(0.22f, 0.302f, 0.282f),
        QVector3D(0.8f, 0.247f, 0.047f)
    };
    Bike& player_bike = m_bikes[0];

    float spawnRadius = m_mapHalfSize * 0.75f;
    float player_baseAngle = 0, jitter = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.4f;
    float player_angle = player_baseAngle + jitter, player_radiusJitter = 0.15f * spawnRadius;
    float r = spawnRadius - player_radiusJitter + (static_cast<float>(std::rand()) / RAND_MAX) * player_radiusJitter;
    float z = std::sin(player_angle) * r;

    player_bike.pos = QVector3D(0, 0.0f, z);
    player_bike.yaw = static_cast<float>(M_PI) - player_angle;
    player_bike.speed = 0.0f;
    player_bike.lean = 0.0f;
    player_bike.color = colors[getColor()];
    player_bike.human = true;
    player_bike.alive = true;
    player_bike.prevPos = player_bike.pos;
    player_bike.currPos = player_bike.pos;
    player_bike.aiTurnTimer = 0.5f + static_cast<float>(std::rand()) / RAND_MAX;
    player_bike.aiTurnDir = 0.0f;
    m_bikeTrails[0].clear();

    TrailPoint player_tp;
    player_tp.pos = player_bike.pos;
    player_tp.time = m_time;
    m_bikeTrails[0].push_back(player_tp);

    for (int i = 1; i < total; ++i) {
        Bike& b = m_bikes[i];
        float baseAngle = (static_cast<float>(i) / total) * 2.0f * static_cast<float>(M_PI);
        jitter = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.4f;
        float angle = baseAngle + jitter, radiusJitter = 0.15f * spawnRadius;
        r = spawnRadius - radiusJitter + (static_cast<float>(std::rand()) / RAND_MAX) * radiusJitter;
        float x = std::cos(angle) * r, z = std::sin(angle) * r;

        b.pos = QVector3D(x, 0.0f, z);
        b.yaw = -angle + static_cast<float>(M_PI); 
        b.speed = 0.0f;
        b.lean = 0.0f;
        b.color = colors[5];
        b.human = (i == 0);
        b.alive = true;
        b.prevPos = b.pos;
        b.currPos = b.pos;
        b.aiTurnTimer = 0.5f + static_cast<float>(std::rand()) / RAND_MAX;
        b.aiTurnDir = 0.0f;
        m_bikeTrails[i].clear();
        
        TrailPoint tp;
        tp.pos = b.pos;
        tp.time = m_time;
        m_bikeTrails[i].push_back(tp);
    }
    if (m_tickTimer)
    m_tickTimer->start(16);
}


void GameProcess::drawBike() {
    float rad2deg = 180.0f / static_cast<float>(M_PI);

    glDisable(GL_BLEND);

    for (size_t i = 0; i < m_bikes.size(); ++i) {
        const Bike& b = m_bikes[i];
        
        if (!b.alive) continue;

        glPushMatrix();
        glTranslatef(b.pos.x(), b.pos.y(), b.pos.z());
        glRotatef(b.yaw * rad2deg, 0.0f, 1.0f, 0.0f);
        glRotatef(b.lean * rad2deg, 0.0f, 0.0f, 1.0f);

        float L = 2.4f, W = 1.2f, H = 1.6f;
        float x0 = -W * 0.5f, x1 = +W * 0.5f, y0 = 0.0f, y1 = H, z0 = -L * 0.5f, z1 = +L * 0.5f;
 
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x0, y1, z0);
        glColor3f(b.color.x() * 0.5f, b.color.y() * 0.5f, b.color.z() * 0.5f);
        glVertex3f(x1, y0, z1);
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z1);
        glVertex3f(x1, y1, z1);
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


void GameProcess::drawTrail() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_CULL_FACE);

    for (size_t i = 0; i < m_bikes.size(); ++i) {
        const Bike& b = m_bikes[i];
        const std::vector<TrailPoint>& trail = m_bikeTrails[i];

        if (trail.size() < 2) continue;

        QVector3D col = b.color;
        float baseR = col.x(), baseG = col.y(), baseB = col.z(), halfWidth = 0.35f, height = 3.0f, baseY = 0.0f;

        for (size_t k = 0; k + 1 < trail.size(); ++k) {
            const TrailPoint& a = trail[k], c = trail[k + 1];
            float ageA = m_time - a.time, ageC = m_time - c.time;

            if (ageA < 0.0f || ageA > m_trailTTL) continue;

            if (ageC < 0.0f || ageC > m_trailTTL) continue;

            float alphaA = 1.0f - ageA / m_trailTTL, alphaC = 1.0f - ageC / m_trailTTL;

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

    while (a > static_cast<float>(M_PI)) a -= twoPi;

    return a;
}

unsigned short GameProcess::getColor() const {
    QJsonObject root = loadConfigRoot();
    QJsonObject player = root.value("player").toObject();
    QString chosen_color = player.value("color").toString();

    if (chosen_color == "leaf") return 0;
    else if (chosen_color == "marine") return 1;
    else if (chosen_color == "dark") return 2;
    else if (chosen_color == "pink") return 3;
    else if (chosen_color == "grey") return 4;
    else return 5; // let it be red for debugging purposes
}