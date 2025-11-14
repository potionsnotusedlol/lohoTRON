#include "GameProcess.h"
#include <QApplication>
#include <QTime>
#include <iostream>

// Helper to clear dialogs
static void ClearCenterDialog(OgreBites::TrayManager* tm)
{
    if (!tm) return;
    const char* ids[] = {
        "pause_title", "btn_resume", "btn_restart", "btn_exit",
        "end_title", "end_winner", "end_score", "end_time"
    };
    for (const char* id : ids) {
        if (tm->getWidget(id)) tm->destroyWidget(id);
    }
}

GameProcess::GameProcess(QWidget* parent) : QOpenGLWidget(parent) {
    std::srand(unsigned(std::time(nullptr)));
    
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    mTimer = new QTimer(this);
    connect(mTimer, &QTimer::timeout, this, [this]() { update(); });
    
    mSpawnsStatic = { Vector3(0,0,0), Vector3(20,0,20), Vector3(-25,0,-15), Vector3(-15,0,30) };
}

GameProcess::~GameProcess() {
    if (mTimer) mTimer->stop();
    
    // Cleanup Ogre
    if (mRoot) {
        mRoot->destroySceneManager(mSceneManager);
        delete mRoot;
    }
}

void GameProcess::buttonHit(Button* b) {
    const String& id = b->getName();
    if (id == "btn_resume") { 
        resumeFromPause(); 
    }
    else if (id == "btn_restart") {
        if (mState == GameState::Paused) { 
            hidePauseDialog(); 
            startNewRoundRandom(); 
        }
        else if (mState == GameState::GameEnd) { 
            hideGameEndDialog(); 
            startMatchFresh(); 
        }
    }
    else if (id == "btn_exit") {
        QApplication::quit();
    }
}

void GameProcess::initializeGL() {
    initializeOpenGLFunctions();
    
    try {
        setupOgre();
        mTimer->start(16); // ~60 FPS
        mLastTime = QDateTime::currentMSecsSinceEpoch();
    } catch (const Ogre::Exception& e) {
        std::cerr << "OGRE ERROR: " << e.getFullDescription() << std::endl;
    }
}

void GameProcess::setupOgre() {
    // Create Root
    mRoot = new Ogre::Root();
    
    // Setup render system
    auto* renderSystem = mRoot->getRenderSystemByName("OpenGL 3+ Rendering Subsystem");
    if (!renderSystem) {
        throw Ogre::Exception(0, "OpenGL 3+ render system not found", "GameProcess::setupOgre");
    }
    mRoot->setRenderSystem(renderSystem);
    mRoot->initialise(false);
    
    // Create window using Qt widget
    Ogre::NameValuePairList params;
    params["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)winId());
    params["parentWindowHandle"] = Ogre::StringConverter::toString((size_t)parentWidget()->winId());
    params["FSAA"] = "0";
    
    mRenderWindow = mRoot->createRenderWindow("OgreRenderWindow", width(), height(), false, &params);
    
    // Create SceneManager
    mSceneManager = mRoot->createSceneManager();
    
    // RTSS
    if (!Ogre::RTShader::ShaderGenerator::getSingletonPtr()) {
        Ogre::RTShader::ShaderGenerator::initialize();
    }
    auto* sg = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    sg->addSceneManager(mSceneManager);
    Ogre::MaterialManager::getSingleton().setActiveScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
    
    // Camera
    mCamera = mSceneManager->createCamera("MainCamera");
    mCamera->setNearClipDistance(0.1f);
    mCamera->setFarClipDistance(2000.0f);
    mCamera->setAutoAspectRatio(true);
    Ogre::Viewport* vp = mRenderWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0.0f, 0.0f, 0.03f));
    
    // TPS rig
    mCamPivot = mSceneManager->getRootSceneNode()->createChildSceneNode("CamPivot");
    mCamYawNode = mCamPivot->createChildSceneNode("CamYaw");
    mCamPitchNode = mCamYawNode->createChildSceneNode("CamPitch");
    mCamEye = mCamPitchNode->createChildSceneNode("CamEye");
    mCamEye->attachObject(mCamera);
    mCamYawNode->setOrientation(Ogre::Quaternion(Ogre::Radian(mCamYaw), Ogre::Vector3::UNIT_Y));
    mCamPitchNode->setOrientation(Ogre::Quaternion(Ogre::Radian(mCamPitch), Ogre::Vector3::UNIT_X));
    mCamEye->setPosition(0, 0, mCamDistCurrent);
    
    // Lighting
    {
        Ogre::Light* dir = mSceneManager->createLight("MainLight");
        dir->setType(Ogre::Light::LT_DIRECTIONAL);
        Ogre::SceneNode* ln = mSceneManager->getRootSceneNode()->createChildSceneNode();
        ln->attachObject(dir);
        ln->setDirection(Ogre::Vector3(-0.4f, -1.0f, -0.25f).normalisedCopy());
        dir->setDiffuseColour(Ogre::ColourValue(0.8f, 0.8f, 0.85f));
        
        Ogre::Light* sky = mSceneManager->createLight("SkyGlow");
        sky->setType(Ogre::Light::LT_POINT);
        Ogre::SceneNode* sn = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 60, 0));
        sn->attachObject(sky);
        sky->setDiffuseColour(Ogre::ColourValue(0.10f, 0.22f, 0.7f));
        sky->setSpecularColour(Ogre::ColourValue(0.10f, 0.22f, 0.7f));
    }
    
    createGrid();
    
    // UI - исправленный вызов для Ogre 14.4
    mTrayMgr = new OgreBites::TrayManager("HUD", mRenderWindow);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    mTrayMgr->hideCursor();
    
    startMatchFresh();
}

void GameProcess::resizeGL(int w, int h) {
    if (mRenderWindow) {
        mRenderWindow->resize(w, h);
        mRenderWindow->windowMovedOrResized();
        
        if (mCamera) {
            mCamera->setAspectRatio(float(w) / float(h));
        }
    }
}

void GameProcess::paintGL() {
    if (!mRoot) return;
    
    // Calculate delta time
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    float dt = mLastTime > 0 ? (currentTime - mLastTime) / 1000.0f : 0.016f;
    mLastTime = currentTime;
    
    updateGame(dt);
    mRoot->renderOneFrame();
}

void GameProcess::updateGame(float dt) {
    mGameTime += dt;
    
    if (mState == GameState::Paused || mState == GameState::GameEnd) {
        updateCameraRig(dt);
        return;
    }
    
    // Start positions
    for (auto& P : mPlayers) if (P.alive) { 
        P.framePrevPos = P.node->getPosition(); 
        P.hitWallThisFrame = false; 
    }
    
    // Movement
    for (auto& P : mPlayers) if (P.alive) {
        if (P.human) updateMove(P, dt, mForward, mBackward, mLeft, mRight);
        else { updateAI(P, dt); updateMove(P, dt, true, false, P.aiTurnDir > 0, P.aiTurnDir < 0); }
        P.frameNewPos = P.node->getPosition();
    }
    
    // Collisions
    resolveCollisionsAndDeaths();
    
    // Check round end conditions
    if (!mPlayers[mHumanIndex].alive) {
        ++mBotsWins; ++mRoundIndex;
        if (mBotsWins >= mRoundsToWin) { 
            showGameEndDialog(false, mGameTime - mRoundStartTime); 
            return; 
        }
        startNewRoundRandom(); 
        return;
    }
    
    if (botsAliveCount() == 0) {
        ++mPlayerWins; ++mRoundIndex;
        if (mPlayerWins >= mRoundsToWin) { 
            showGameEndDialog(true, mGameTime - mRoundStartTime); 
            return; 
        }
        startNewRoundRandom(); 
        return;
    }
    
    // Trails
    for (auto& P : mPlayers) if (P.alive) updateTrail(P);
    
    // Camera
    updateCameraRig(dt);
}

// Input handlers
void GameProcess::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_W: mForward = true; break;
        case Qt::Key_S: mBackward = true; break;
        case Qt::Key_A: mLeft = true; break;
        case Qt::Key_D: mRight = true; break;
        case Qt::Key_Escape: 
            if (mState == GameState::Playing) showPauseDialog();
            else if (mState == GameState::Paused) resumeFromPause();
            break;
        default: break;
    }
}

void GameProcess::keyReleaseEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_W: mForward = false; break;
        case Qt::Key_S: mBackward = false; break;
        case Qt::Key_A: mLeft = false; break;
        case Qt::Key_D: mRight = false; break;
        default: break;
    }
}

void GameProcess::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        mRmbDown = true;
        setCursor(Qt::BlankCursor);
    }
}

void GameProcess::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        mRmbDown = false;
        setCursor(Qt::ArrowCursor);
    }
}

void GameProcess::mouseMoveEvent(QMouseEvent* event) {
    if (mRmbDown) {
        QPoint delta = event->pos() - mLastMousePos;
        mCamYaw -= float(delta.x()) * mMouseSensitivity;
        mCamPitch -= float(delta.y()) * mMouseSensitivity;
        mCamPitch = clampf(mCamPitch, Ogre::Degree(-85).valueRadians(), Ogre::Degree(85).valueRadians());
        mCamYawNode->setOrientation(Ogre::Quaternion(Ogre::Radian(mCamYaw), Ogre::Vector3::UNIT_Y));
        mCamPitchNode->setOrientation(Ogre::Quaternion(Ogre::Radian(mCamPitch), Ogre::Vector3::UNIT_X));
        
        // Keep cursor centered for continuous rotation
        QCursor::setPos(mapToGlobal(QPoint(width()/2, height()/2)));
        mLastMousePos = QPoint(width()/2, height()/2);
    } else {
        mLastMousePos = event->pos();
    }
}

void GameProcess::wheelEvent(QWheelEvent* event) {
    mCamDistance = clampf(mCamDistance - float(event->angleDelta().y()) * 0.01f, mCamMinDist, mCamMaxDist);
}

// Game methods implementation
void GameProcess::createGrid() {
    // Ground material
    {
        Ogre::MaterialPtr gmat = Ogre::MaterialManager::getSingleton().create(
            "GroundMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass* p = gmat->getTechnique(0)->getPass(0);
        p->setLightingEnabled(true);
        p->setDiffuse(Ogre::ColourValue(0.015f, 0.02f, 0.05f));
        p->setAmbient(Ogre::ColourValue(0.01f, 0.01f, 0.02f));
        p->setSelfIllumination(Ogre::ColourValue(0.03f, 0.03f, 0.08f));
    }
    
    // Grid material
    {
        Ogre::MaterialPtr lmat = Ogre::MaterialManager::getSingleton().create(
            "GridMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass* p = lmat->getTechnique(0)->getPass(0);
        p->setLightingEnabled(false);
        p->setDiffuse(Ogre::ColourValue(0.0f, 0.6f, 1.0f));
        p->setSelfIllumination(Ogre::ColourValue(0.0f, 0.55f, 1.2f));
        p->setSceneBlending(Ogre::SBT_ADD);
        p->setDepthWriteEnabled(false);
    }
    
    // Ground plane
    Ogre::ManualObject* ground = mSceneManager->createManualObject("Ground");
    ground->begin("GroundMaterial", Ogre::RenderOperation::OT_TRIANGLE_LIST);
    float half = mMapHalfSize;
    for (int x = 0; x < mGridSize; ++x)
    for (int z = 0; z < mGridSize; ++z) {
        float x0 = x * mCellSize - half, x1 = x0 + mCellSize;
        float z0 = z * mCellSize - half, z1 = z0 + mCellSize;
        Ogre::Vector3 v0(x0,0,z0), v1(x1,0,z0), v2(x1,0,z1), v3(x0,0,z1);
        size_t base = ground->getCurrentVertexCount();
        ground->position(v0); ground->normal(Ogre::Vector3::UNIT_Y);
        ground->position(v1); ground->normal(Ogre::Vector3::UNIT_Y);
        ground->position(v2); ground->normal(Ogre::Vector3::UNIT_Y);
        ground->position(v3); ground->normal(Ogre::Vector3::UNIT_Y);
        ground->triangle(base+0,base+1,base+2);
        ground->triangle(base+0,base+2,base+3);
    }
    ground->end();
    mSceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(ground);
    
    // Grid lines
    Ogre::ManualObject* grid = mSceneManager->createManualObject("GridLines");
    grid->begin("GridMaterial", Ogre::RenderOperation::OT_LINE_LIST);
    for (int i = 0; i <= mGridSize; ++i) {
        float p = i * mCellSize - half;
        grid->position(-half, 0.02f, p); grid->position(+half, 0.02f, p);
        grid->position(p, 0.02f, -half); grid->position(p, 0.02f, +half);
    }
    grid->end();
    mSceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(grid);
}

Ogre::Entity* GameProcess::createBoxEntity(const Ogre::String& meshName, const Ogre::String& entName,
                                          float L, float W, float H, const Ogre::ColourValue& color) {
    Ogre::String matName = entName + "_Mat";
    {
        Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().create(
            matName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass* p = mat->getTechnique(0)->getPass(0);
        p->setLightingEnabled(true);
        p->setDiffuse(color);
        p->setAmbient(color * 0.35f);
        p->setSelfIllumination(color * 0.65f);
    }
    
    Ogre::ManualObject* m = mSceneManager->createManualObject(entName + "_MO");
    m->begin(matName, Ogre::RenderOperation::OT_TRIANGLE_LIST);
    
    Ogre::Vector3 p[8] = {
        {-L*0.5f, 0.0f, -W*0.5f}, {-L*0.5f, 0.0f,  W*0.5f},
        {-L*0.5f, H,     W*0.5f}, {-L*0.5f, H,    -W*0.5f},
        { L*0.5f, 0.0f, -W*0.5f}, { L*0.5f, 0.0f,  W*0.5f},
        { L*0.5f, H,     W*0.5f}, { L*0.5f, H,    -W*0.5f}
    };
    auto addQuad=[&](int a,int b,int c,int d){
        Ogre::Vector3 n=(p[b]-p[a]).crossProduct(p[c]-p[a]); n.normalise();
        size_t base=m->getCurrentVertexCount();
        m->position(p[a]); m->normal(n);
        m->position(p[b]); m->normal(n);
        m->position(p[c]); m->normal(n);
        m->position(p[d]); m->normal(n);
        m->triangle(base+0,base+1,base+2);
        m->triangle(base+0,base+2,base+3);
    };
    addQuad(0,1,2,3); addQuad(4,7,6,5); addQuad(0,3,7,4);
    addQuad(1,5,6,2); addQuad(0,4,5,1); addQuad(3,2,6,7);
    m->end();
    
    Ogre::MeshPtr mesh = m->convertToMesh(meshName);
    mSceneManager->destroyManualObject(m);
    
    Ogre::Entity* ent = mSceneManager->createEntity(entName, meshName);
    ent->setMaterialName(matName);
    return ent;
}

void GameProcess::spawnPlayer(bool human, const Ogre::Vector3& start, const Ogre::ColourValue& color, const Ogre::String& name) {
    Player P;
    P.human = human; 
    P.color = Ogre::ColourValue(color.r, color.g, color.b, 1.0f);
    P.name  = name;  
    P.alive = true;
    
    P.ent  = createBoxEntity(name+"_BoxMesh", name+"_BoxEntity", 2.5f, 1.6f, 1.6f, P.color);
    P.node = mSceneManager->getRootSceneNode()->createChildSceneNode(name + "_Node");
    P.node->attachObject(P.ent);
    
    Ogre::AxisAlignedBox bb = P.ent->getBoundingBox();
    float raise = -bb.getMinimum().y;
    P.node->translate(0, raise, 0);
    
    P.node->setPosition(start);
    P.yaw = 0.0f; P.speed = 0.0f; P.lean = 0.0f;
    P.node->setOrientation(Ogre::Quaternion(Ogre::Radian(P.yaw), Ogre::Vector3::UNIT_Y));
    
    // Light
    P.glow = mSceneManager->createLight(name + "_Glow");
    P.glow->setType(Ogre::Light::LT_POINT);
    P.glow->setDiffuseColour(P.color * 1.6f);
    P.glow->setSpecularColour(P.color);
    P.glow->setAttenuation(140.0f, 1.0f, 0.004f, 0.0004f);
    P.node->createChildSceneNode(name+"_GlowNode", Ogre::Vector3(0, 1.0f, 0))->attachObject(P.glow);
    
    // Trail material
    {
        Ogre::MaterialPtr m = Ogre::MaterialManager::getSingleton().create(
            name + "_TrailMat", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass* ps = m->getTechnique(0)->getPass(0);
        ps->setLightingEnabled(false);
        ps->setDiffuse(P.color);
        ps->setAmbient(Ogre::ColourValue::Black);
        ps->setSelfIllumination(P.color);
        ps->setSceneBlending(Ogre::SBT_ADD);
        ps->setDepthWriteEnabled(false);
    }
    
    // Trail chain
    P.trail = mSceneManager->createBillboardChain(name + "_Trail");
    P.trail->setNumberOfChains(1);
    P.trail->setMaxChainElements(1024);
    P.trail->setUseTextureCoords(false);
    P.trail->setUseVertexColours(true);
    P.trail->setMaterialName(name + "_TrailMat");
    mSceneManager->getRootSceneNode()->createChildSceneNode(name + "_TrailNode")->attachObject(P.trail);
    
    if (!P.human) { 
        P.aiTurnTimer = frand(0.5f, 2.0f); 
        P.aiTurnDir = (frand(0.0f,1.0f)>0.5f?+1.0f:-1.0f); 
    }
    
    P.lastTrailPos = start;
    P.framePrevPos = start;
    P.frameNewPos  = start;
    
    mPlayers.push_back(P);
}

float GameProcess::frand(float a, float b) { 
    std::uniform_real_distribution<float> d(a,b); 
    return d(mRng); 
}

void GameProcess::startMatchFresh() {
    mPlayerWins = mBotsWins = 0;
    mRoundIndex = 1;
    
    mPlayers.clear();
    spawnPlayer(true,  mSpawnsStatic[0], Ogre::ColourValue(0.2f, 0.7f, 1.0f), "Player");
    
    // Создаем ботов в соответствии с настройками
    for (int i = 0; i < mNumberOfBots; ++i) {
        Ogre::ColourValue botColor;
        Ogre::String botName;
        
        switch(i % 3) {
            case 0: 
                botColor = Ogre::ColourValue(1.0f, 0.3f, 0.3f);
                botName = "Bot_Red_" + Ogre::StringConverter::toString(i);
                break;
            case 1:
                botColor = Ogre::ColourValue(0.2f, 1.0f, 0.4f);
                botName = "Bot_Green_" + Ogre::StringConverter::toString(i);
                break;
            case 2:
                botColor = Ogre::ColourValue(1.0f, 0.8f, 0.2f);
                botName = "Bot_Yellow_" + Ogre::StringConverter::toString(i);
                break;
        }
        
        spawnPlayer(false, mSpawnsStatic[(i % 3) + 1], botColor, botName);
    }
    
    mHumanIndex = 0;
    mRoundStartTime = mGameTime;
    mState = GameState::Playing;
}

void GameProcess::startNewRoundRandom() {
    const int N = (int)mPlayers.size();
    std::vector<Ogre::Vector3> spawns = generateRandomSpawns(N);
    
    for (int i=0;i<N;++i) {
        Player& P = mPlayers[i];
        P.alive = true; P.speed = 0.0f; P.yaw = frand(-Ogre::Math::PI, Ogre::Math::PI); P.lean = 0.0f;
        
        Ogre::Vector3 start = spawns[i];
        P.node->setPosition(start);
        P.node->setOrientation(Ogre::Quaternion(Ogre::Radian(P.yaw), Ogre::Vector3::UNIT_Y));
        P.node->setVisible(true, true);
        
        P.framePrevPos = start;
        P.frameNewPos  = start;
        
        P.pts.clear(); P.trail->clearChain(0); P.lastTrailPos = start;
        
        if (!P.human) { 
            P.aiTurnTimer = frand(0.5f, 2.0f); 
            P.aiTurnDir = (frand(0.0f,1.0f)>0.5f?+1.0f:-1.0f); 
        }
    }
    
    mRoundStartTime = mGameTime;
    mState = GameState::Playing;
}

std::vector<Ogre::Vector3> GameProcess::generateRandomSpawns(int count) {
    std::vector<Ogre::Vector3> out; 
    out.reserve(count);
    float border = mMapHalfSize - 6.0f;
    float minDist = 10.0f;
    int tries = 0;
    while ((int)out.size() < count && tries < 2000) {
        ++tries;
        Ogre::Vector3 p(frand(-border,border), 0.0f, frand(-border,border));
        bool ok=true; 
        for (auto& q: out) if ((p-q).length() < minDist) { ok=false; break; }
        if (ok) out.push_back(p);
    }
    while ((int)out.size() < count) 
        out.push_back(mSpawnsStatic[out.size()%mSpawnsStatic.size()]);
    return out;
}

void GameProcess::updateMove(Player& P, float dt, bool W, bool S, bool A, bool D) {
    float acc = 0.0f;
    if (W) acc += mAcceleration;
    if (S) acc += (P.speed > 0.0f ? -mBrakeDecel : -mAcceleration);
    
    if (W || S) P.speed += acc * dt;
    else {
        if (P.speed > 0.0f) { P.speed -= mFriction * dt; if (P.speed < 0) P.speed = 0; }
        else if (P.speed < 0.0f) { P.speed += mFriction * dt; if (P.speed > 0) P.speed = 0; }
    }
    P.speed = clampf(P.speed, -mMaxBackwardSpeed, mMaxForwardSpeed);
    
    float turn = (A ? 1.0f : 0.0f) + (D ? -1.0f : 0.0f);
    float dirSign = (P.speed >= 0.0f ? 1.0f : -1.0f);
    P.yaw += turn * mTurnSpeed * dt * dirSign;
    
    float speedFactor = std::min(1.0f, std::fabs(P.speed) / mMaxForwardSpeed);
    float targetLean = turn * mMaxLeanAngle * speedFactor;
    P.lean = lerpf(P.lean, targetLean, std::min(1.0f, mLeanSpeed * dt));
    
    Ogre::Quaternion yawQ(Ogre::Radian(P.yaw), Ogre::Vector3::UNIT_Y);
    Ogre::Quaternion rollQ(Ogre::Radian(P.lean), Ogre::Vector3::UNIT_Z);
    P.node->setOrientation(yawQ * rollQ);
    
    Ogre::Vector3 forward = yawQ * Ogre::Vector3(0, 0, -1);
    Ogre::Vector3 cand    = P.node->getPosition() + forward * (P.speed * dt);
    
    float border = mMapHalfSize - 2.0f;
    P.hitWallThisFrame = (std::fabs(cand.x) > border) || (std::fabs(cand.z) > border);
    Ogre::Vector3 newPos(cand.x, cand.y, cand.z);
    newPos.x = clampf(newPos.x, -border, border);
    newPos.z = clampf(newPos.z, -border, border);
    P.node->setPosition(newPos);
}

void GameProcess::updateAI(Player& P, float dt) {
    P.aiTurnTimer -= dt;
    if (P.aiTurnTimer <= 0.0f) {
        P.aiTurnTimer = frand(0.7f, 2.0f);
        P.aiTurnDir   = (frand(0.0f, 1.0f) > 0.5f ? +1.0f : -1.0f);
    }
    Ogre::Vector3 pos = P.node->getPosition();
    float edge = mMapHalfSize * 0.85f;
    if (std::fabs(pos.x) > edge || std::fabs(pos.z) > edge) {
        Ogre::Vector3 toCenter = -pos;
        Ogre::Vector3 fwd = (Ogre::Quaternion(Ogre::Radian(P.yaw), Ogre::Vector3::UNIT_Y) * Ogre::Vector3(0,0,-1));
        float side = fwd.crossProduct(toCenter).y;
        P.aiTurnDir = (side > 0 ? +1.0f : -1.0f);
        P.aiTurnTimer = 0.3f;
    }
}

void GameProcess::updateTrail(Player& P) {
    const Ogre::Vector3 pos = P.node->getPosition() + Ogre::Vector3(0, 1.0f, 0);
    if (P.pts.empty() || (pos - P.lastTrailPos).length() >= mTrailMinSegDist) {
        P.pts.push_back({pos, mGameTime});
        P.lastTrailPos = pos;
    }
    while (!P.pts.empty() && (mGameTime - P.pts.front().t) > mTrailTTL) 
        P.pts.pop_front();
    
    P.trail->clearChain(0);
    for (const auto& tp : P.pts) {
        float age = mGameTime - tp.t;
        float a   = clampf(1.0f - age / mTrailTTL, 0.0f, 1.0f);
        Ogre::ColourValue c = P.color; c.a = a;
        Ogre::BillboardChain::Element el(tp.pos, mTrailWidth, 0.0f, c, Ogre::Quaternion::IDENTITY);
        P.trail->addChainElement(0, el);
    }
}

void GameProcess::resolveCollisionsAndDeaths() {
    const float trailRad = mTrailWidth * 0.5f;
    const float epsAlpha = 1e-4f;
    
    std::vector<Hit> hits;
    
    for (size_t i = 0; i < mPlayers.size(); ++i) {
        Player& A = mPlayers[i];
        if (!A.alive) continue;
        
        Ogre::Vector2 a0(A.framePrevPos.x, A.framePrevPos.z);
        Ogre::Vector2 a1(A.frameNewPos.x,  A.frameNewPos.z);
        
        float bestAlpha = std::numeric_limits<float>::infinity();
        bool  any = false;
        
        // Wall collision
        if (A.hitWallThisFrame) { any = true; bestAlpha = std::min(bestAlpha, 1.0f); }
        
        // Trail collisions
        for (size_t j = 0; j < mPlayers.size(); ++j) {
            const Player& B = mPlayers[j];
            size_t segCount = (B.pts.size() >= 2) ? (B.pts.size() - 1) : 0;
            if (segCount == 0) continue;
            
            size_t limit = segCount;
            if (j == i) { 
                if (limit > mSelfSkipSegments) limit -= mSelfSkipSegments; 
                else limit = 0; 
            }
            
            for (size_t k = 0; k < limit; ++k) {
                Ogre::Vector2 b0(B.pts[k].pos.x,   B.pts[k].pos.z);
                Ogre::Vector2 b1(B.pts[k+1].pos.x, B.pts[k+1].pos.z);
                Closest2D cl = closestSegSeg2D(a0, a1, b0, b1);
                if (j == i && cl.t <= mSelfTouchEps) continue;
                float r = mPlayerRadius + trailRad;
                if (cl.dist2 <= r*r) { any = true; if (cl.t < bestAlpha) bestAlpha = cl.t; }
            }
        }
        
        // Player-to-player collisions
        for (size_t j = 0; j < mPlayers.size(); ++j) if (j != i) {
            const Player& B = mPlayers[j]; if (!B.alive) continue;
            Ogre::Vector2 b0(B.framePrevPos.x, B.framePrevPos.z);
            Ogre::Vector2 b1(B.frameNewPos.x,  B.frameNewPos.z);
            Closest2D cl = closestSegSeg2D(a0, a1, b0, b1);
            float r = mPlayerRadius + mPlayerRadius;
            if (cl.dist2 <= r*r) { any = true; if (cl.t < bestAlpha) bestAlpha = cl.t; }
        }
        
        if (any) hits.push_back({ i, bestAlpha, (i==mHumanIndex) });
    }
    
    if (hits.empty()) return;
    
    float best = hits[0].alpha;
    for (auto& h : hits) best = std::min(best, h.alpha);
    
    bool playerAmongBest = false;
    for (auto& h : hits) if (std::fabs(h.alpha - best) <= epsAlpha && h.isPlayer) { 
        playerAmongBest = true; break; 
    }
    
    if (playerAmongBest) {
        killPlayer(mHumanIndex);
        return;
    }
    
    for (auto& h : hits) if (std::fabs(h.alpha - best) <= epsAlpha) killPlayer(h.idx);
}

void GameProcess::killPlayer(size_t idx) {
    Player& P = mPlayers[idx];
    if (!P.alive) return;
    P.alive = false;
    if (P.node) P.node->setVisible(false, true);
}

int GameProcess::botsAliveCount() const {
    int n = 0;
    for (size_t i=0;i<mPlayers.size();++i) 
        if (i!=mHumanIndex && mPlayers[i].alive) ++n;
    return n;
}

float GameProcess::wrapPi(float a) { 
    while (a >  Ogre::Math::PI) a -= Ogre::Math::TWO_PI; 
    while (a < -Ogre::Math::PI) a += Ogre::Math::TWO_PI; 
    return a; 
}

void GameProcess::updateCameraRig(float dt) {
    if (mPlayers.empty()) return;
    Player& H = mPlayers[mHumanIndex];
    Ogre::Vector3 target = H.node->getPosition() + Ogre::Vector3(0, mCamTargetHeight, 0);
    mCamPivot->setPosition(target);
    if (!mRmbDown) {
        float diff = wrapPi(H.yaw - mCamYaw);
        float t = 1.0f - std::exp(-mCamFollowYawSmooth * dt);
        mCamYaw += diff * t;
        mCamYawNode->setOrientation(Ogre::Quaternion(Ogre::Radian(mCamYaw), Ogre::Vector3::UNIT_Y));
    }
    float tz = 1.0f - std::exp(-mCamSmooth * dt);
    mCamDistCurrent += (mCamDistance - mCamDistCurrent) * tz;
    mCamPitchNode->setOrientation(Ogre::Quaternion(Ogre::Radian(mCamPitch), Ogre::Vector3::UNIT_X));
    mCamEye->setPosition(0, 0, mCamDistCurrent);
}

// UI Methods
void GameProcess::showPauseDialog() {
    if (!mTrayMgr) return;
    mState = GameState::Paused;
    ClearCenterDialog(mTrayMgr);
    mTrayMgr->createLabel (OgreBites::TL_CENTER, "pause_title", "Пауза", 250);
    mTrayMgr->createButton(OgreBites::TL_CENTER, "btn_resume",  "Возобновить игру", 250);
    mTrayMgr->createButton(OgreBites::TL_CENTER, "btn_restart", "Рестарт (случайные позиции)", 250);
    mTrayMgr->createButton(OgreBites::TL_CENTER, "btn_exit",    "Выйти из игры", 250);
}

void GameProcess::hidePauseDialog() {
    if (!mTrayMgr) return;
    ClearCenterDialog(mTrayMgr);
    mState = GameState::Playing;
}

void GameProcess::resumeFromPause() {
    hidePauseDialog();
    mState = GameState::Playing;
}

void GameProcess::showGameEndDialog(bool playerWon, float roundTime) {
    mState = GameState::GameEnd;
    if (!mTrayMgr) return;
    ClearCenterDialog(mTrayMgr);
    
    Ogre::String title = "Игра окончена";
    Ogre::String info  = playerWon ? "Победитель: " + mPlayers[mHumanIndex].name
                                 : "Победитель: Боты";
    
    mTrayMgr->createLabel(OgreBites::TL_CENTER, "end_title", title, 300);
    mTrayMgr->createLabel(OgreBites::TL_CENTER, "end_winner", info, 300);
    
    Ogre::String score = "Счёт: " + Ogre::StringConverter::toString(mPlayerWins) + " : " + Ogre::StringConverter::toString(mBotsWins);
    mTrayMgr->createLabel(OgreBites::TL_CENTER, "end_score", score, 300);
    
    Ogre::String timeStr = "Время раунда: " + Ogre::StringConverter::toString(roundTime, 1) + " c";
    mTrayMgr->createLabel(OgreBites::TL_CENTER, "end_time", timeStr, 300);
    
    mTrayMgr->createButton(OgreBites::TL_CENTER, "btn_restart", "Рестарт матча", 250);
    mTrayMgr->createButton(OgreBites::TL_CENTER, "btn_exit",    "Выйти", 250);
}

void GameProcess::hideGameEndDialog() {
    if (!mTrayMgr) return;
    ClearCenterDialog(mTrayMgr);
    mState = GameState::Playing;
}