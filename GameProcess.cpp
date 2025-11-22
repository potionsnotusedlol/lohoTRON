#include "GameProcess.h"
#include <QApplication>
#include <QTime>
#include <QDebug>
#include <QCursor>
#include <QDateTime>


class SGTechniqueResolverListener : public Ogre::MaterialManager::Listener
{
public:
    explicit SGTechniqueResolverListener(Ogre::RTShader::ShaderGenerator* shaderGen)
        : mShaderGenerator(shaderGen)
    {}

    Ogre::Technique* handleSchemeNotFound(
        unsigned short /*schemeIndex*/,
        const Ogre::String& schemeName,
        Ogre::Material* originalMaterial,
        unsigned short /*lodIndex*/,
        const Ogre::Renderable* /*rend*/) override
    {
        if (schemeName != Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
            return nullptr;

        Ogre::Technique* bestTech = originalMaterial->getBestTechnique();
        if (!bestTech)
            return nullptr;

        bool created = mShaderGenerator->createShaderBasedTechnique(
            *originalMaterial,
            bestTech->getSchemeName(),
            schemeName
        );

        if (!created)
            return nullptr;

        unsigned short techCount = originalMaterial->getNumTechniques();
        for (unsigned short i = 0; i < techCount; i++) {
            Ogre::Technique* t = originalMaterial->getTechnique(i);
            if (t->getSchemeName() == schemeName)
                return t;
        }
        return nullptr;
    }

private:
    Ogre::RTShader::ShaderGenerator* mShaderGenerator;
};



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

GameProcess::GameProcess(QWidget* parent)
    : QWidget(parent)
    , mRoot(nullptr)
    , mRenderWindow(nullptr)
    , mSceneManager(nullptr)
    , mCamera(nullptr)
    , mViewport(nullptr)
    , mShaderGenerator(nullptr)
    , mMaterialMgrListener(nullptr)
    , mMaterialListener(nullptr)
    , mTrayMgr(nullptr)
    , mCamPivot(nullptr)
    , mCamYawNode(nullptr)
    , mCamPitchNode(nullptr)
    , mCamEye(nullptr)
    , mCamYaw(0.0f)
    , mCamPitch(-0.6f)
    , mCamDistance(40.0f)
    , mCamDistCurrent(40.0f)
    , mCamTargetHeight(4.0f)
    , mCamSmooth(6.0f)
    , mCamFollowYawSmooth(5.0f)
    , mMouseSensitivity(0.005f)
    , mOgreInitialised(false)
    , mReadyToRender(false)
    , mSceneCreated(false)
    , mState(GameState::Playing)
    , mGameTime(0.0f)
    , mRoundStartTime(0.0f)
    , mRoundIndex(1)
    , mHumanIndex(0)
    , mNumberOfBots(3)
    , mPlayerWins(0)
    , mBotsWins(0)
    , mRoundsToWin(3)
    , mForward(false)
    , mBackward(false)
    , mLeft(false)
    , mRight(false)
    , mRmbDown(false)
    , mTimer(new QTimer(this))
    , mLastTime(0)
{
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setUpdatesEnabled(true);
    
    // Подключаем таймер (но НЕ запускаем!)
    connect(mTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    
    // Инициализация игровых параметров
    mMapHalfSize = 60.0f;
    mGridSize = 24;
    mCellSize = (mMapHalfSize * 2.0f) / float(mGridSize);
    mPlayerRadius = 1.0f;
    mTrailWidth = 1.2f;
    mTrailTTL = 4.0f;
    mTrailMinSegDist = 0.5f;
    mSelfSkipSegments = 10;
    mSelfTouchEps = 0.02f;
    
    mAcceleration = 30.0f;
    mBrakeDecel = 40.0f;
    mFriction = 15.0f;
    mMaxForwardSpeed = 40.0f;
    mMaxBackwardSpeed = 10.0f;
    mTurnSpeed = Ogre::Degree(120).valueRadians();
    mMaxLeanAngle = Ogre::Degree(20).valueRadians();
    mLeanSpeed = 8.0f;
    
    // Статические позиции спавна
    mSpawnsStatic.clear();
    mSpawnsStatic.push_back(Ogre::Vector3(0, 0, 30));
    mSpawnsStatic.push_back(Ogre::Vector3(30, 0, 0));
    mSpawnsStatic.push_back(Ogre::Vector3(0, 0, -30));
    mSpawnsStatic.push_back(Ogre::Vector3(-30, 0, 0));
    
    // Инициализация генератора случайных чисел
    mRng.seed(uint32_t(QDateTime::currentMSecsSinceEpoch() & 0xffffffff));
    
    std::cout << "[CTOR] GameProcess created" << std::endl;
}



GameProcess::~GameProcess() {
    std::cout << "GameProcess destructor called" << std::endl;
    
    if (mTimer) {
        mTimer->stop();
    }
    
    // Удаляем listener
    if (mMaterialMgrListener) {
        Ogre::MaterialManager::getSingleton().removeListener(mMaterialMgrListener);
        delete mMaterialMgrListener;
        mMaterialMgrListener = nullptr;
    }
    
    // Останавливаем RTSS
    if (mShaderGenerator) {
        if (mSceneManager) {
            mShaderGenerator->removeSceneManager(mSceneManager);
        }
        Ogre::RTShader::ShaderGenerator::destroy();
        mShaderGenerator = nullptr;
    }
    
    // Удаляем Root (автоматически очистит всё остальное)
    if (mRoot) {
        delete mRoot;
        mRoot = nullptr;
    }
}


// отключаем QPainter — рисуем через OGRE
QPaintEngine* GameProcess::paintEngine() const
{
    return nullptr;
}

// TrayListener
void GameProcess::buttonHit(Button* b)
{
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
        emit returnToMenuRequested();
    }
}

void GameProcess::createScene() {
    std::cout << "\n--- createScene START ---" << std::endl;
    
    if (!mSceneManager) {
        std::cerr << "[SCENE ERROR] mSceneManager is NULL!" << std::endl;
        return;
    }
    
    std::cout << "[SCENE] Calling setupLighting()..." << std::endl;
    setupLighting();
    std::cout << "[SCENE] setupLighting() OK" << std::endl;
    
    std::cout << "[SCENE] Calling createGrid()..." << std::endl;
    createGrid();
    std::cout << "[SCENE] createGrid() OK" << std::endl;
    
    std::cout << "--- createScene END ---\n" << std::endl;
}

void GameProcess::setupLighting() {
    if (!mSceneManager) {
        std::cerr << "Cannot setup lighting: SceneManager is null" << std::endl;
        return;
    }
    
    // Проверяем, не создано ли уже освещение
    if (mSceneManager->hasLight("MainLight")) {
        std::cout << "Lighting already set up, skipping..." << std::endl;
        return;
    }
    
    std::cout << "Setting up lighting..." << std::endl;
    
    // Направленный свет
    Ogre::Light* dirLight = mSceneManager->createLight("MainLight");
    dirLight->setType(Ogre::Light::LT_DIRECTIONAL);
    Ogre::SceneNode* lightNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
    lightNode->attachObject(dirLight);
    lightNode->setDirection(Ogre::Vector3(-0.4f, -1.0f, -0.25f).normalisedCopy());
    dirLight->setDiffuseColour(Ogre::ColourValue(0.8f, 0.8f, 0.85f));
    
    // Небесное освещение
    Ogre::Light* skyLight = mSceneManager->createLight("SkyGlow");
    skyLight->setType(Ogre::Light::LT_POINT);
    Ogre::SceneNode* skyNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 60, 0));
    skyNode->attachObject(skyLight);
    skyLight->setDiffuseColour(Ogre::ColourValue(0.10f, 0.22f, 0.7f));
    skyLight->setSpecularColour(Ogre::ColourValue(0.10f, 0.22f, 0.7f));
    
    std::cout << "Lighting created!" << std::endl;
}

void GameProcess::initializeOgre() {
    std::cout << "\n========== INITIALIZE OGRE START ==========" << std::endl;
    
    if (mOgreInitialised) {
        std::cout << "[INIT] Already initialized" << std::endl;
        return;
    }
    
    try {
        std::cout << "[INIT] Step 1: Calling setupOgre()..." << std::endl;
        setupOgre();
        std::cout << "[INIT] Step 1: setupOgre() completed" << std::endl;
        
        // Добавляем ТОЛЬКО освещение
        std::cout << "[INIT] Step 2: Adding lights..." << std::endl;
        setupLighting();
        std::cout << "[INIT] Step 2: Lights added" << std::endl;
        
        mOgreInitialised = true;
        std::cout << "[INIT] OGRE READY (lights only)" << std::endl;
        std::cout << "========== INITIALIZE OGRE END (SUCCESS) ==========\n" << std::endl;
        
    } catch (const Ogre::Exception& e) {
        std::cerr << "\n[INIT ERROR] OGRE Exception: " << e.getFullDescription() << std::endl;
        mOgreInitialised = false;
    } catch (const std::exception& e) {
        std::cerr << "\n[INIT ERROR] Exception: " << e.what() << std::endl;
        mOgreInitialised = false;
    }
}




void GameProcess::setupOgre() {
    std::cout << "\n--- setupOgre START ---" << std::endl;
    
    if (mRoot) {
        std::cout << "[SETUP] mRoot already exists, skipping" << std::endl;
        return;
    }
    
    std::cout << "[SETUP] Creating Root..." << std::endl;
    mRoot = new Ogre::Root("plugins.cfg", "ogre.cfg", "ogre.log");
    std::cout << "[SETUP] Root created OK" << std::endl;
    
    std::cout << "[SETUP] Getting render systems..." << std::endl;
    const Ogre::RenderSystemList& rsList = mRoot->getAvailableRenderers();
    if (rsList.empty()) {
        throw std::runtime_error("No render systems available");
    }
    std::cout << "[SETUP] Found " << rsList.size() << " render systems" << std::endl;
    
    Ogre::RenderSystem* rs = rsList[0];
    std::cout << "[SETUP] Setting render system..." << std::endl;
    mRoot->setRenderSystem(rs);
    std::cout << "[SETUP] Render system set OK" << std::endl;
    
    std::cout << "[SETUP] Initializing Root (false)..." << std::endl;
    mRoot->initialise(false);
    std::cout << "[SETUP] Root initialized OK" << std::endl;
    
    std::cout << "[SETUP] Getting winId: " << winId() << std::endl;
    WId wid = winId();
    if (wid == 0) {
        throw std::runtime_error("Invalid winId!");
    }
    
    std::cout << "[SETUP] Creating RenderWindow params..." << std::endl;
    Ogre::NameValuePairList params;
    params["parentWindowHandle"] = Ogre::StringConverter::toString((unsigned long)wid);
    
    std::cout << "[SETUP] Creating RenderWindow " << width() << "x" << height() << "..." << std::endl;
    mRenderWindow = mRoot->createRenderWindow("GameWindow", width(), height(), false, &params);
    std::cout << "[SETUP] RenderWindow created OK" << std::endl;
    
    mRenderWindow->setActive(true);
    mRenderWindow->setVisible(true);
    std::cout << "[SETUP] RenderWindow activated" << std::endl;
    
    std::cout << "[SETUP] Creating SceneManager..." << std::endl;
    mSceneManager = mRoot->createSceneManager("DefaultSceneManager", "MainSceneManager");
    mSceneManager->setAmbientLight(Ogre::ColourValue(0.05f, 0.05f, 0.1f));
    std::cout << "[SETUP] SceneManager created OK" << std::endl;
    
    std::cout << "[SETUP] Creating Camera..." << std::endl;
    mCamera = mSceneManager->createCamera("MainCamera");
    mCamera->setNearClipDistance(0.1f);
    mCamera->setFarClipDistance(1000.0f);
    mCamera->setAutoAspectRatio(true);
    std::cout << "[SETUP] Camera created OK" << std::endl;
    
    std::cout << "[SETUP] Creating Viewport..." << std::endl;
    mViewport = mRenderWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(Ogre::ColourValue(0.0f, 0.0f, 0.03f));
    std::cout << "[SETUP] Viewport created OK" << std::endl;
    
    std::cout << "[SETUP] Initializing RTSS..." << std::endl;
    if (Ogre::RTShader::ShaderGenerator::initialize()) {
        mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
        mShaderGenerator->addSceneManager(mSceneManager);
        mViewport->setMaterialScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
        
        mMaterialMgrListener = new SGTechniqueResolverListener(mShaderGenerator);
        Ogre::MaterialManager::getSingleton().addListener(mMaterialMgrListener);
        
        std::cout << "[SETUP] RTSS initialized successfully" << std::endl;
    } else {
        std::cerr << "[SETUP] RTSS initialization FAILED!" << std::endl;
    }
    
    std::cout << "[SETUP] Creating camera rig..." << std::endl;
    mCamPivot = mSceneManager->getRootSceneNode()->createChildSceneNode("CamPivot");
    mCamYawNode = mCamPivot->createChildSceneNode("CamYaw");
    mCamPitchNode = mCamYawNode->createChildSceneNode("CamPitch");
    mCamEye = mCamPitchNode->createChildSceneNode("CamEye");
    mCamEye->attachObject(mCamera);
    
    mCamYawNode->setOrientation(Ogre::Quaternion(Ogre::Radian(mCamYaw), Ogre::Vector3::UNIT_Y));
    mCamPitchNode->setOrientation(Ogre::Quaternion(Ogre::Radian(mCamPitch), Ogre::Vector3::UNIT_X));
    mCamEye->setPosition(0, 0, mCamDistCurrent);
    std::cout << "[SETUP] Camera rig created OK" << std::endl;
    
    std::cout << "--- setupOgre END ---\n" << std::endl;
}


void GameProcess::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    int w = event->size().width();
    int h = event->size().height();
    std::cout << "Resizing to: " << w << "x" << h << std::endl;

    if (mRenderWindow) {
        mRenderWindow->resize(w, h);
        mRenderWindow->windowMovedOrResized();

        if (mCamera && h > 0) {
            mCamera->setAspectRatio(float(w) / float(h));
        }
    }
}


void GameProcess::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    std::cout << "\n========== SHOWEVENT START ==========" << std::endl;
    std::cout << "[SHOW] Widget size: " << width() << "x" << height() << std::endl;
    std::cout << "[SHOW] mOgreInitialised: " << mOgreInitialised << std::endl;
    
    if (mOgreInitialised) {
        std::cout << "[SHOW] Already initialized, skipping" << std::endl;
        std::cout << "========== SHOWEVENT END ==========\n" << std::endl;
        return;
    }
    
    std::cout << "[SHOW] Scheduling initializeOgre in 50ms..." << std::endl;
    QTimer::singleShot(50, this, [this]() {
        std::cout << "[SHOW TIMER] Fired, calling initializeOgre..." << std::endl;
        initializeOgre();
    });
    
    std::cout << "========== SHOWEVENT END ==========\n" << std::endl;
}



void GameProcess::paintEvent(QPaintEvent* /*event*/) {
    static int frameCount = 0;
    
    if (frameCount < 5) {
        std::cout << "\n========== PAINT Frame " << frameCount << " ==========" << std::endl;
        std::cout << "[PAINT] mReadyToRender: " << mReadyToRender << std::endl;
    }
    
    if (!mReadyToRender) {
        if (frameCount < 3) {
            std::cout << "[PAINT] Not ready, skip\n" << std::endl;
        }
        return;
    }
    
    if (!mOgreInitialised || !mRoot || !mRenderWindow) {
        std::cout << "[PAINT] OGRE not initialized, skip\n" << std::endl;
        return;
    }
    
    if (!mRenderWindow->isActive() || mRenderWindow->isClosed()) {
        std::cout << "[PAINT] Window not active, skip\n" << std::endl;
        return;
    }
    
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    float dt = (mLastTime > 0) ? (now - mLastTime) / 1000.0f : 0.016f;
    dt = std::min(dt, 0.1f);
    mLastTime = now;
    
    try {
        // ТЕСТ: первые 5 кадров БЕЗ updateGame
        if (frameCount < 5) {
            std::cout << "[PAINT] TEST RENDER #" << frameCount << " WITHOUT updateGame" << std::endl;
            std::cout << "[PAINT] Calling renderOneFrame()..." << std::endl;
            mRoot->renderOneFrame();
            std::cout << "[PAINT] Test render SUCCESS!" << std::endl;
            std::cout << "========== PAINT Frame " << frameCount << " END ==========\n" << std::endl;
            frameCount++;
            return; // <--- КРИТИЧНО: выходим до updateGame
        }
        
        // После 5 кадров включаем полную логику
        if (frameCount == 5) {
            std::cout << "\n========================================" << std::endl;
            std::cout << "[PAINT] Starting FULL LOGIC from frame 5" << std::endl;
            std::cout << "========================================\n" << std::endl;
        }
        
        std::cout << "[PAINT] Frame " << frameCount << " with updateGame" << std::endl;
        updateGame(dt);
        mRoot->renderOneFrame();
        
    } catch (const Ogre::Exception& e) {
        std::cerr << "\n[PAINT ERROR] OGRE Exception at frame " << frameCount << ": " 
                  << e.getFullDescription() << std::endl;
        mTimer->stop();
        mReadyToRender = false;
    } catch (const std::exception& e) {
        std::cerr << "\n[PAINT ERROR] Exception at frame " << frameCount << ": " 
                  << e.what() << std::endl;
        mTimer->stop();
        mReadyToRender = false;
    } catch (...) {
        std::cerr << "\n[PAINT ERROR] Unknown exception at frame " << frameCount << std::endl;
        mTimer->stop();
        mReadyToRender = false;
    }
    
    frameCount++;
}





void GameProcess::focusInEvent(QFocusEvent* event)
{
    QWidget::focusInEvent(event);
    std::cout << "GameProcess gained focus" << std::endl;
}

void GameProcess::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
    std::cout << "GameProcess lost focus" << std::endl;
}

// Input handlers

void GameProcess::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_W: mForward  = true; break;
        case Qt::Key_S: mBackward = true; break;
        case Qt::Key_A: mLeft     = true; break;
        case Qt::Key_D: mRight    = true; break;
        case Qt::Key_Escape:
            if (mState == GameState::Playing)      showPauseDialog();
            else if (mState == GameState::Paused)  resumeFromPause();
            else if (mState == GameState::GameEnd) emit returnToMenuRequested();
            break;
        default: break;
    }
}

void GameProcess::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_W: mForward  = false; break;
        case Qt::Key_S: mBackward = false; break;
        case Qt::Key_A: mLeft     = false; break;
        case Qt::Key_D: mRight    = false; break;
        default: break;
    }
}

void GameProcess::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        mRmbDown = true;
        setCursor(Qt::BlankCursor);
        QCursor::setPos(mapToGlobal(QPoint(width()/2, height()/2)));
        mLastMousePos = QPoint(width()/2, height()/2);
    }
}

void GameProcess::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        mRmbDown = false;
        setCursor(Qt::ArrowCursor);
    }
}

void GameProcess::mouseMoveEvent(QMouseEvent* event)
{
    if (mRmbDown) {
        QPoint delta = event->pos() - mLastMousePos;
        mCamYaw   -= float(delta.x()) * mMouseSensitivity;
        mCamPitch -= float(delta.y()) * mMouseSensitivity;
        mCamPitch = clampf(mCamPitch,
                           Ogre::Degree(-85).valueRadians(),
                           Ogre::Degree(85).valueRadians());
        mCamYawNode->setOrientation(
            Ogre::Quaternion(Ogre::Radian(mCamYaw), Ogre::Vector3::UNIT_Y));
        mCamPitchNode->setOrientation(
            Ogre::Quaternion(Ogre::Radian(mCamPitch), Ogre::Vector3::UNIT_X));

        QCursor::setPos(mapToGlobal(QPoint(width()/2, height()/2)));
        mLastMousePos = QPoint(width()/2, height()/2);
    } else {
        mLastMousePos = event->pos();
    }
}

void GameProcess::wheelEvent(QWheelEvent* event)
{
    mCamDistance = clampf(
        mCamDistance - float(event->angleDelta().y()) * 0.01f,
        mCamMinDist, mCamMaxDist);
}

// ===== ИГРОВАЯ ЛОГИКА (взята из твоего исходника) =====
void GameProcess::updateCameraRig(float dt) {
    static int camUpdateCount = 0;
    
    if (camUpdateCount < 3) {
        std::cout << "[CAMERA #" << camUpdateCount << "] Updating camera rig, dt=" << dt << std::endl;
    }
    
    // КРИТИЧНЫЕ ПРОВЕРКИ
    if (!mCamPivot || !mCamYawNode || !mCamPitchNode || !mCamEye) {
        if (camUpdateCount < 3) {
            std::cerr << "[CAMERA ERROR] Camera rig nodes are NULL!" << std::endl;
        }
        return;
    }
    
    if (mPlayers.empty()) {
        if (camUpdateCount < 3) {
            std::cout << "[CAMERA] No players, skip" << std::endl;
        }
        return;
    }
    
    if (mHumanIndex >= mPlayers.size()) {
        std::cerr << "[CAMERA ERROR] Invalid mHumanIndex!" << std::endl;
        return;
    }
    
    if (!mPlayers[mHumanIndex].node) {
        std::cerr << "[CAMERA ERROR] Player node is NULL!" << std::endl;
        return;
    }
    
    // Ваша основная логика updateCameraRig
    // ...
    
    camUpdateCount++;
}


void GameProcess::updateGame(float dt) {
    static int updateCount = 0;
    
    if (updateCount < 3) {
        std::cout << "[UPDATE #" << updateCount << "] dt=" << dt 
                  << ", players=" << mPlayers.size() << std::endl;
    }
    
    // Базовые проверки
    if (!mRoot || !mSceneManager || !mCamera) {
        if (updateCount < 3) {
            std::cout << "[UPDATE] OGRE not ready, skip" << std::endl;
        }
        return;
    }
    
    if (mPlayers.empty()) {
        if (updateCount < 3) {
            std::cout << "[UPDATE] No players, skip" << std::endl;
        }
        return;
    }
    
    if (mHumanIndex >= mPlayers.size()) {
        std::cerr << "[UPDATE ERROR] Invalid mHumanIndex" << std::endl;
        return;
    }
    
    dt = std::min(dt, 0.1f);
    mGameTime += dt;
    
    // ВРЕМЕННО: МИНИМАЛЬНАЯ ЛОГИКА - ТОЛЬКО КАМЕРА
    if (updateCount < 3) {
        std::cout << "[UPDATE] Updating camera only (minimal logic)" << std::endl;
    }
    
    // Проверяем, что камера pivot существует
    if (mCamPivot && mPlayers[mHumanIndex].node) {
        Ogre::Vector3 playerPos = mPlayers[mHumanIndex].node->getPosition();
        mCamPivot->setPosition(playerPos.x, 0, playerPos.z);
        
        if (updateCount < 3) {
            std::cout << "[UPDATE] Camera follows player at " << playerPos << std::endl;
        }
    }
    
    updateCount++;
}


// ===== дальше твои вспомогательные методы без изменений =====

void GameProcess::createGrid() {
    if (!mSceneManager) return;
    
    std::cout << "Creating grid..." << std::endl;
    
    // === МАТЕРИАЛ С ЗАЩИТОЙ ОТ ПОВТОРНОГО СОЗДАНИЯ ===
    Ogre::MaterialPtr mat;
    if (Ogre::MaterialManager::getSingleton().resourceExists("GroundMaterial")) {
        mat = Ogre::MaterialManager::getSingleton().getByName("GroundMaterial");
        std::cout << "GroundMaterial already exists, reusing..." << std::endl;
    } else {
        mat = Ogre::MaterialManager::getSingleton().create("GroundMaterial", 
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        mat->getTechnique(0)->getPass(0)->setDiffuse(0.05f, 0.08f, 0.15f, 1.0f);
        mat->getTechnique(0)->getPass(0)->setAmbient(0.05f, 0.08f, 0.15f);
        mat->getTechnique(0)->getPass(0)->setSelfIllumination(0.02f, 0.04f, 0.1f);
        std::cout << "GroundMaterial created" << std::endl;
    }
    
    // === ПРОВЕРКА: не создан ли уже Ground ===
    if (mSceneManager->hasManualObject("Ground")) {
        std::cout << "Ground already exists, skipping..." << std::endl;
        return;
    }
    
    // === СОЗДАНИЕ ПОЛА ===
    Ogre::ManualObject* ground = mSceneManager->createManualObject("Ground");
    ground->begin("GroundMaterial", Ogre::RenderOperation::OT_LINE_LIST);
    
    float gridSize = 200.0f;
    int gridLines = 40;
    float step = gridSize / gridLines;
    
    for (int i = 0; i <= gridLines; ++i) {
        float pos = -gridSize / 2.0f + i * step;
        // Линии по X
        ground->position(pos, 0, -gridSize / 2.0f);
        ground->position(pos, 0, gridSize / 2.0f);
        // Линии по Z
        ground->position(-gridSize / 2.0f, 0, pos);
        ground->position(gridSize / 2.0f, 0, pos);
    }
    
    ground->end();
    
    Ogre::SceneNode* groundNode = mSceneManager->getRootSceneNode()->createChildSceneNode("GroundNode");
    groundNode->attachObject(ground);
    
    std::cout << "Grid created!" << std::endl;
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

void GameProcess::spawnPlayer(bool human, const Ogre::Vector3& start,
                              const Ogre::ColourValue& color, const Ogre::String& name)
{
    std::cout << "\n[SPAWN] Starting spawn for: " << name << std::endl;
    
    // Проверяем инициализацию сцены
    if (!mSceneManager) {
        std::cerr << "[SPAWN ERROR] SceneManager is null!" << std::endl;
        return;
    }
    
    try {
        std::cout << "[SPAWN] Creating Player structure..." << std::endl;
        Player newPlayer;
        newPlayer.human = human;
        newPlayer.color = Ogre::ColourValue(color.r, color.g, color.b, 1.0f);
        newPlayer.name = name;
        newPlayer.alive = true;
        
        // Создаем модель игрока
        std::cout << "[SPAWN] Creating box entity..." << std::endl;
        newPlayer.ent = createBoxEntity(name + "_BoxMesh", name + "_BoxEntity", 
                                       2.5f, 1.6f, 1.6f, newPlayer.color);
        if (!newPlayer.ent) {
            throw std::runtime_error("Failed to create player entity");
        }
        std::cout << "[SPAWN] Box entity created: " << newPlayer.ent->getName() << std::endl;
        
        // Создаем узел
        std::cout << "[SPAWN] Creating scene node..." << std::endl;
        newPlayer.node = mSceneManager->getRootSceneNode()->createChildSceneNode(name + "_Node");
        if (!newPlayer.node) {
            throw std::runtime_error("Failed to create player node");
        }
        std::cout << "[SPAWN] Node created: " << newPlayer.node->getName() << std::endl;
        
        // Прикрепляем entity к node
        std::cout << "[SPAWN] Attaching entity to node..." << std::endl;
        newPlayer.node->attachObject(newPlayer.ent);
        std::cout << "[SPAWN] Entity attached, objects on node: " << newPlayer.node->numAttachedObjects() << std::endl;
        
        // Корректируем позицию чтобы модель стояла на земле
        Ogre::AxisAlignedBox bb = newPlayer.ent->getBoundingBox();
        float raise = -bb.getMinimum().y;
        newPlayer.node->translate(0, raise, 0);
        
        // Устанавливаем начальную позицию и ориентацию
        newPlayer.node->setPosition(start);
        newPlayer.yaw = 0.0f;
        newPlayer.speed = 0.0f;
        newPlayer.lean = 0.0f;
        newPlayer.node->setOrientation(Ogre::Quaternion(Ogre::Radian(newPlayer.yaw), Ogre::Vector3::UNIT_Y));
        std::cout << "[SPAWN] Position set to: " << newPlayer.node->getPosition() << std::endl;
        
        // Создаем свечение
        std::cout << "[SPAWN] Creating glow light..." << std::endl;
        newPlayer.glow = mSceneManager->createLight(name + "_Glow");
        if (!newPlayer.glow) {
            throw std::runtime_error("Failed to create player glow light");
        }
        newPlayer.glow->setType(Ogre::Light::LT_POINT);
        newPlayer.glow->setDiffuseColour(newPlayer.color * 1.6f);
        newPlayer.glow->setSpecularColour(newPlayer.color);
        newPlayer.glow->setAttenuation(140.0f, 1.0f, 0.004f, 0.0004f);
        
        Ogre::SceneNode* glowNode = newPlayer.node->createChildSceneNode(name + "_GlowNode", Ogre::Vector3(0, 1.0f, 0));
        glowNode->attachObject(newPlayer.glow);
        std::cout << "[SPAWN] Glow light created and attached" << std::endl;
        
        // Создаем материал для следа
        std::cout << "[SPAWN] Creating trail material..." << std::endl;
        Ogre::String trailMatName = name + "_TrailMat";
        Ogre::MaterialPtr trailMat;
        
        if (Ogre::MaterialManager::getSingleton().resourceExists(trailMatName)) {
            std::cout << "[SPAWN] Trail material already exists, reusing..." << std::endl;
            trailMat = Ogre::MaterialManager::getSingleton().getByName(trailMatName);
        } else {
            trailMat = Ogre::MaterialManager::getSingleton().create(
                trailMatName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            Ogre::Pass* pass = trailMat->getTechnique(0)->getPass(0);
            pass->setLightingEnabled(false);
            pass->setDiffuse(newPlayer.color);
            pass->setAmbient(Ogre::ColourValue::Black);
            pass->setSelfIllumination(newPlayer.color);
            pass->setSceneBlending(Ogre::SBT_ADD);
            pass->setDepthWriteEnabled(false);
            std::cout << "[SPAWN] Trail material created" << std::endl;
        }
        
        // Создаем след
        std::cout << "[SPAWN] Creating trail..." << std::endl;
        newPlayer.trail = mSceneManager->createBillboardChain(name + "_Trail");
        if (!newPlayer.trail) {
            throw std::runtime_error("Failed to create trail");
        }
        newPlayer.trail->setNumberOfChains(1);
        newPlayer.trail->setMaxChainElements(1024);
        newPlayer.trail->setUseTextureCoords(false);
        newPlayer.trail->setUseVertexColours(true);
        newPlayer.trail->setMaterialName(trailMatName);
        
        Ogre::SceneNode* trailNode = mSceneManager->getRootSceneNode()->createChildSceneNode(name + "_TrailNode");
        trailNode->attachObject(newPlayer.trail);
        std::cout << "[SPAWN] Trail created and attached" << std::endl;
        
        // Настраиваем ИИ для ботов
        if (!newPlayer.human) {
            newPlayer.aiTurnTimer = frand(0.5f, 2.0f);
            newPlayer.aiTurnDir = (frand(0.0f, 1.0f) > 0.5f ? +1.0f : -1.0f);
            std::cout << "[SPAWN] AI initialized for bot" << std::endl;
        }
        
        // Инициализируем позиции для коллизий
        newPlayer.lastTrailPos = start;
        newPlayer.framePrevPos = start;
        newPlayer.frameNewPos = start;
        
        // Добавляем игрока в список
        mPlayers.push_back(newPlayer);
        
        std::cout << "[SPAWN] SUCCESS: " << name << " spawned at " << start << std::endl;
        std::cout << "[SPAWN] Total players now: " << mPlayers.size() << std::endl;
        std::cout << "[SPAWN] Player details:" << std::endl;
        std::cout << "  - node: " << newPlayer.node->getName() << std::endl;
        std::cout << "  - entity: " << newPlayer.ent->getName() << std::endl;
        std::cout << "  - attached objects: " << newPlayer.node->numAttachedObjects() << std::endl;
        std::cout << "  - position: " << newPlayer.node->getPosition() << std::endl;
        std::cout << "[SPAWN] End\n" << std::endl;
        
    } catch (const Ogre::Exception& e) {
        std::cerr << "\n[SPAWN ERROR] OGRE Exception for '" << name << "': " 
                  << e.getFullDescription() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\n[SPAWN ERROR] Exception for '" << name << "': " 
                  << e.what() << std::endl;
    } catch (...) {
        std::cerr << "\n[SPAWN ERROR] Unknown exception for '" << name << "'" << std::endl;
    }
}


float GameProcess::frand(float a, float b) {
    std::uniform_real_distribution<float> d(a,b);
    return d(mRng);
}

void GameProcess::startMatchFresh() {
    std::cout << "\n--- startMatchFresh START ---" << std::endl;
    
    if (!mSceneManager) {
        std::cerr << "[MATCH ERROR] SceneManager is NULL!" << std::endl;
        return;
    }
    
    if (!mCamera) {
        std::cerr << "[MATCH ERROR] Camera is NULL!" << std::endl;
        return;
    }
    
    std::cout << "[MATCH] Clearing old players..." << std::endl;
    mPlayers.clear();
    
    std::cout << "[MATCH] Preparing spawn positions..." << std::endl;
    std::vector<Ogre::Vector3> positions = mSpawnsStatic;
    std::vector<std::string> names = {"Player", "Bot_Red", "Bot_Green", "Bot_Yellow"};
    std::vector<Ogre::ColourValue> colors = {
        Ogre::ColourValue(0.2f, 0.6f, 1.0f),   // Синий
        Ogre::ColourValue(1.0f, 0.2f, 0.2f),   // Красный
        Ogre::ColourValue(0.2f, 1.0f, 0.2f),   // Зелёный
        Ogre::ColourValue(1.0f, 1.0f, 0.2f)    // Жёлтый
    };
    
    int toSpawn = std::min((int)positions.size(), 1 + mNumberOfBots);
    
    std::cout << "[MATCH] Spawning " << toSpawn << " players..." << std::endl;
    for (int i = 0; i < toSpawn; i++) {
        bool isHuman = (i == 0);
        std::string name = (i < names.size()) ? names[i] : ("Bot_" + std::to_string(i));
        Ogre::ColourValue color = (i < colors.size()) ? colors[i] : Ogre::ColourValue::White;
        Ogre::Vector3 pos = positions[i];
        
        std::cout << "[MATCH] Spawning #" << i << ": " << name << " at " << pos << std::endl;
        spawnPlayer(isHuman, pos, color, name);
    }
    
    std::cout << "[MATCH] Total players spawned: " << mPlayers.size() << std::endl;
    std::cout << "[MATCH] Verifying players..." << std::endl;
    
    // Упрощенная проверка - только node
    for (size_t i = 0; i < mPlayers.size(); i++) {
        if (!mPlayers[i].node) {
            std::cerr << "[MATCH ERROR] Player " << i << " has NO NODE!" << std::endl;
        } else {
            std::cout << "[MATCH] Player " << i << " (" << mPlayers[i].name 
                      << ") OK at " << mPlayers[i].node->getPosition() << std::endl;
        }
    }
    
    mRoundStartTime = mGameTime;
    std::cout << "--- startMatchFresh END (players: " << mPlayers.size() << ") ---\n" << std::endl;
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

        if (A.hitWallThisFrame) { any = true; bestAlpha = std::min(bestAlpha, 1.0f); }

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

void GameProcess::showPauseDialog() {
    if (!mTrayMgr) return;
    mState = GameState::Paused;
    ClearCenterDialog(mTrayMgr);
    mTrayMgr->createLabel (OgreBites::TL_CENTER, "pause_title", "Пауза", 250);
    mTrayMgr->createButton(OgreBites::TL_CENTER, "btn_resume",  "Возобновить игру", 250);
    mTrayMgr->createButton(OgreBites::TL_CENTER, "btn_restart", "Рестарт (случайные позиции)", 250);
    mTrayMgr->createButton(OgreBites::TL_CENTER, "btn_exit",    "Выйти в меню", 250);
}

void GameProcess::activateGame() {
    std::cout << "\n========== ACTIVATEGAME START ==========" << std::endl;
    std::cout << "[ACTIVATE] mOgreInitialised: " << mOgreInitialised << std::endl;
    
    if (!mOgreInitialised || !mRenderWindow) {
        std::cerr << "[ACTIVATE ERROR] OGRE not ready!" << std::endl;
        return;
    }
    
    std::cout << "[ACTIVATE] TEST MODE: No players, rendering empty scene" << std::endl;
    
    std::cout << "[ACTIVATE] Setting focus..." << std::endl;
    setFocus();
    grabKeyboard();
    grabMouse();
    setCursor(Qt::BlankCursor);
    
    QPoint center = mapToGlobal(rect().center());
    QCursor::setPos(center);
    mLastMousePos = rect().center();
    
    if (!mTimer->isActive()) {
        std::cout << "[ACTIVATE] Starting timer..." << std::endl;
        mLastTime = QDateTime::currentMSecsSinceEpoch();
        mTimer->start(16);
        std::cout << "[ACTIVATE] Timer started!" << std::endl;
    }
    
    mReadyToRender = true;
    std::cout << "[ACTIVATE] mReadyToRender = TRUE (empty scene test)" << std::endl;
    std::cout << "========== ACTIVATEGAME END ==========\n" << std::endl;
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

    Ogre::String score = "Счёт: " + Ogre::StringConverter::toString(mPlayerWins)
                         + " : " + Ogre::StringConverter::toString(mBotsWins);
    mTrayMgr->createLabel(OgreBites::TL_CENTER, "end_score", score, 300);

    Ogre::String timeStr = "Время раунда: " + Ogre::StringConverter::toString(roundTime, 1) + " c";
    mTrayMgr->createLabel(OgreBites::TL_CENTER, "end_time", timeStr, 300);

    mTrayMgr->createButton(OgreBites::TL_CENTER, "btn_restart", "Рестарт матча", 250);
    mTrayMgr->createButton(OgreBites::TL_CENTER, "btn_exit",    "Выйти в меню", 250);
}

void GameProcess::hideGameEndDialog() {
    if (!mTrayMgr) return;
    ClearCenterDialog(mTrayMgr);
    mState = GameState::Playing;
}