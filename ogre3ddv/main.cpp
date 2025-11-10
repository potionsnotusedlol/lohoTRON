#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>
#include <OgreManualObject.h>
#include <OgreMeshManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreCamera.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreLight.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>

#include <SDL2/SDL_keycode.h>

#include <iostream>
#include <vector>
#include <deque>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <string>

using namespace Ogre;
using namespace OgreBites;

class SimpleApp :
        public ApplicationContext,
        public InputListener,
{
public:
    SimpleApp()
        : ApplicationContext("TronBikeDemo14")
        , mWindow(nullptr)
        , mSceneManager(nullptr)
        , mCamera(nullptr)
        , mViewport(nullptr)
        , mCameraYaw(0.0f)
        , mCameraPitch(0.4f)
        , mMouseSensitivity(0.002f)
        , mGridSize(80)
        , mCellSize(2.0f)
        , mForward(false)
        , mBackward(false)
        , mLeft(false)
        , mRight(false)
        , mPlayerNode(nullptr)
        , mPlayerEntity(nullptr)
        , mCameraDistance(18.0f)
        , mCameraHeight(6.0f)
        , mTrailLifetime(15.0f)
        , mTrailUpdateInterval(0.05f)
        , mTrailLastUpdate(0.0f)
        , mTrailWidth(0.8f)
        , mTrailHeight(3.0f)
        , mTrailSegmentCounter(0)
        , mGameTime(0.0f)
        , mPlayerYaw(0.0f)
        , mCurrentSpeed(0.0f)
        , mMaxForwardSpeed(40.0f)
        , mMaxBackwardSpeed(20.0f)
        , mAcceleration(45.0f)
        , mBrakeDecel(70.0f)
        , mFriction(25.0f)
        , mTurnSpeed(2.8f)
        , mCurrentLean(0.0f)
        , mMaxLeanAngle(Degree(35.0f).valueRadians())
        , mLeanSpeed(6.0f)
    {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        mMapHalfSize = (mGridSize * mCellSize) * 0.5f;
        mPlayerLastTrailPos = Vector3::ZERO;
    }

    ~SimpleApp() override
    {
        clearTrail(mPlayerTrail);
        for (auto &e : mEnemies) clearTrail(e.trail);
    }

    // === OgreBites::ApplicationContext ===
    void setup() override
    {
        ApplicationContext::setup();
        addInputListener(this);

        // Окно / базовые объекты
        mWindow = getRenderWindow();
        Root* root = getRoot();

        // Ресурсы (добавим локальные папки поверх resources.cfg)
        ResourceGroupManager::getSingleton().addResourceLocation("./", "FileSystem");
        ResourceGroupManager::getSingleton().addResourceLocation("./media", "FileSystem");
        ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        // Сцена
        mSceneManager = root->createSceneManager();
        // RTSS (нужно для GL3+ материалов)
        if (!RTShader::ShaderGenerator::getSingletonPtr())
        {
            RTShader::ShaderGenerator::initialize();
        }
        auto* sg = RTShader::ShaderGenerator::getSingletonPtr();
        sg->addSceneManager(mSceneManager);
        MaterialManager::getSingleton().setActiveScheme(RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

        // Камера/вьюпорт
        mCamera = mSceneManager->createCamera("MainCamera");
        mCamera->setNearClipDistance(0.1f);
        mCamera->setFarClipDistance(1000.0f);
        mCamera->setAutoAspectRatio(true);
        mViewport = mWindow->addViewport(mCamera);
        mViewport->setBackgroundColour(ColourValue(0.0f, 0.0f, 0.05f));

        createDefaultMaterials();
        createScene();
        createTrailMaterials();
        createPlayer();
        createEnemies(3);

        root->addFrameListener(this);

        std::cout << "=== Tron Bike Demo (OGRE 14.4) ===\n";
        std::cout << "W/S - вперёд/назад\n";
        std::cout << "A/D - поворот (с наклоном)\n";
        std::cout << "Мышь - вращение камеры\n";
        std::cout << "ESC - выход\n";
    }

    // ===================== FrameListener ========================
    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        if (mWindow->isClosed()) return false;

        float dt = evt.timeSinceLastFrame;
        mGameTime += dt;

        updatePlayerMovement(dt);
        updateEnemies(dt);
        updateTrails(dt, mGameTime);
        updateCameraPosition();

        return true;
    }

    // ===================== Ввод (SDL через OgreBites) ===========
    bool keyPressed(const KeyboardEvent& evt) override
    {
        switch (evt.keysym.sym)
        {
            case SDLK_w: mForward  = true; break;
            case SDLK_s: mBackward = true; break;
            case SDLK_a: mLeft     = true; break;
            case SDLK_d: mRight    = true; break;
            default: break;
        }
        return true;
    }

    bool keyReleased(const KeyboardEvent& evt) override
    {
        switch (evt.keysym.sym)
        {
            case SDLK_w: mForward  = false; break;
            case SDLK_s: mBackward = false; break;
            case SDLK_a: mLeft     = false; break;
            case SDLK_d: mRight    = false; break;
            default: break;
        }
        return true;
    }

    bool mouseMoved(const MouseMotionEvent& evt) override
    {
        mCameraYaw   -= static_cast<float>(evt.xrel) * mMouseSensitivity;
        mCameraPitch -= static_cast<float>(evt.yrel) * mMouseSensitivity;

        float limit = Math::HALF_PI - 0.1f;
        if (mCameraPitch > limit)  mCameraPitch = limit;
        if (mCameraPitch < -limit) mCameraPitch = -limit;
        return true;
    }

    bool mousePressed(const MouseButtonEvent&) override { return true; }
    bool mouseReleased(const MouseButtonEvent&) override { return true; }

private:
    // --------- OGRE -------------
    RenderWindow*  mWindow;
    SceneManager*  mSceneManager;
    Camera*        mCamera;
    Viewport*      mViewport;

    // --------- Камера -----------------
    float mCameraYaw;
    float mCameraPitch;
    float mMouseSensitivity;

    // --------- Пол / карта ------------
    int   mGridSize;
    float mCellSize;
    float mMapHalfSize;

    // --------- Ввод игрока ------------
    bool mForward, mBackward, mLeft, mRight;

    // --------- Игрок ------------------
    SceneNode* mPlayerNode;
    Entity*    mPlayerEntity;

    float mPlayerYaw;        // угол поворота вокруг Y (рад)
    float mCurrentSpeed;     // скорость вперёд (+) / назад (-)
    float mMaxForwardSpeed;
    float mMaxBackwardSpeed;
    float mAcceleration;     // ускорение вперёд/назад
    float mBrakeDecel;       // торможение (когда жмём S и едем вперёд)
    float mFriction;         // трение, когда ничего не жмём
    float mTurnSpeed;        // скорость поворота (рад/с при полном отклонении)

    float mCurrentLean;      // текущий наклон мотоцикла (рад)
    float mMaxLeanAngle;     // макс. наклон (рад)
    float mLeanSpeed;        // как быстро байк наклоняется/выпрямляется

    // --------- Камера от 3-го лица ----
    float mCameraDistance;
    float mCameraHeight;

    // --------- Следы -------------------
    struct TrailSegment
    {
        Vector3        position;
        Vector3        prevPosition;
        float          timestamp;
        ManualObject*  object;
        SceneNode*     node;
        bool           isPlayerTrail;
    };

    std::deque<TrailSegment> mPlayerTrail;
    float                    mTrailLifetime;
    float                    mTrailUpdateInterval;
    float                    mTrailLastUpdate;
    float                    mTrailWidth;
    float                    mTrailHeight;
    Vector3                  mPlayerLastTrailPos;
    int                      mTrailSegmentCounter;

    // --------- Враги ------------------
    struct Enemy
    {
        SceneNode* node;
        Entity*    entity;

        Vector3    direction;
        Vector3    velocity;

        float      maxSpeed;
        float      acceleration;
        float      deceleration;

        float      yaw;

        float      directionChangeTimer;
        float      directionChangeInterval;

        std::deque<TrailSegment> trail;
        float                    trailLastUpdate;
        Vector3                  lastTrailPos;
    };

    std::vector<Enemy> mEnemies;

    // --------- Общие таймеры ----------
    float mGameTime;

private:
    // ===================== ИНИЦИАЛИЗАЦИЯ ========================

    void createDefaultMaterials()
    {
        // Простой дефолтный белый материал, чтобы безопасно вызывать ManualObject::begin(...)
        if (MaterialManager::getSingleton().getByName("DefaultWhite").isNull())
        {
            auto mat = MaterialManager::getSingleton().create(
                "DefaultWhite", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            auto* t = mat->getTechnique(0);
            auto* p = t->getPass(0);
            p->setDiffuse(ColourValue::White);
            p->setAmbient(ColourValue(0.5f, 0.5f, 0.5f));
            p->setSelfIllumination(ColourValue(0.0f, 0.0f, 0.0f));
            p->setLightingEnabled(true);
        }
    }

    void createScene()
    {
        mSceneManager->setAmbientLight(ColourValue(0.1f, 0.1f, 0.1f));

        Light* mainLight = mSceneManager->createLight("MainLight");
        mainLight->setType(Light::LT_DIRECTIONAL);
        mainLight->setDirection(Vector3(-0.5f, -1.0f, -0.3f).normalisedCopy());
        mainLight->setDiffuseColour(ColourValue(0.4f, 0.4f, 0.4f));

        Light* neonLight = mSceneManager->createLight("NeonLight");
        neonLight->setType(Light::LT_POINT);
        neonLight->setPosition(0, 30, 0);
        neonLight->setDiffuseColour(ColourValue(0.1f, 0.2f, 0.6f));
        neonLight->setSpecularColour(ColourValue(0.1f, 0.2f, 0.6f));

        createGrid();
        updateCameraPosition();
    }

    void createGrid()
    {
        float halfSize = mMapHalfSize;

        // Материал пола
        MaterialPtr groundMat = MaterialManager::getSingleton().create(
            "GroundMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        auto* gTech = groundMat->getTechnique(0);
        auto* gPass = gTech->getPass(0);
        gPass->setDiffuse(ColourValue(0.03f, 0.03f, 0.08f));
        gPass->setAmbient(ColourValue(0.01f, 0.01f, 0.03f));
        gPass->setLightingEnabled(true);

        // Пол (треугольники)
        ManualObject* ground = mSceneManager->createManualObject("Ground");
        ground->begin("GroundMaterial", RenderOperation::OT_TRIANGLE_LIST);

        for (int x = 0; x < mGridSize; ++x)
        {
            for (int z = 0; z < mGridSize; ++z)
            {
                float x0 = x * mCellSize - halfSize;
                float x1 = x0 + mCellSize;
                float z0 = z * mCellSize - halfSize;
                float z1 = z0 + mCellSize;

                Vector3 v0(x0, 0, z0);
                Vector3 v1(x1, 0, z0);
                Vector3 v2(x1, 0, z1);
                Vector3 v3(x0, 0, z1);

                size_t baseIndex = ground->getCurrentVertexCount();

                ground->position(v0); ground->normal(Vector3::UNIT_Y);
                ground->position(v1); ground->normal(Vector3::UNIT_Y);
                ground->position(v2); ground->normal(Vector3::UNIT_Y);
                ground->position(v3); ground->normal(Vector3::UNIT_Y);

                ground->triangle(baseIndex + 0, baseIndex + 1, baseIndex + 2);
                ground->triangle(baseIndex + 0, baseIndex + 2, baseIndex + 3);
            }
        }

        ground->end();

        SceneNode* groundNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
        groundNode->attachObject(ground);

        // Материал сетки
        MaterialPtr gridMat = MaterialManager::getSingleton().create(
            "GridMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        auto* t = gridMat->getTechnique(0);
        auto* p = t->getPass(0);
        p->setDiffuse(ColourValue(0.0f, 0.5f, 1.0f));
        p->setAmbient(ColourValue(0.0f, 0.15f, 0.3f));
        p->setSelfIllumination(ColourValue(0.0f, 0.4f, 1.0f));
        p->setLightingEnabled(true);

        // Неоновая сетка (линии)
        ManualObject* grid = mSceneManager->createManualObject("GridLines");
        grid->begin("GridMaterial", RenderOperation::OT_LINE_LIST);
        for (int i = 0; i <= mGridSize; ++i)
        {
            float pos = i * mCellSize - halfSize;

            grid->position(-halfSize, 0.02f, pos);
            grid->position( halfSize, 0.02f, pos);

            grid->position(pos, 0.02f, -halfSize);
            grid->position(pos, 0.02f,  halfSize);
        }
        grid->end();

        SceneNode* gridNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
        gridNode->attachObject(grid);
    }

    // ============== Материалы для следов ==============
    void createTrailMaterials()
    {
        MaterialPtr playerTrailMat = MaterialManager::getSingleton().create(
            "PlayerTrailMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        playerTrailMat->setReceiveShadows(false);
        auto* t = playerTrailMat->getTechnique(0);
        auto* p = t->getPass(0);
        p->setDiffuse(ColourValue(0.0f, 0.8f, 1.0f, 0.8f));
        p->setAmbient(ColourValue(0.0f, 0.3f, 0.4f));
        p->setSelfIllumination(ColourValue(0.0f, 0.6f, 0.8f));
        p->setSceneBlending(SBT_TRANSPARENT_ALPHA);
        p->setDepthWriteEnabled(false);
        p->setLightingEnabled(true);

        MaterialPtr enemyTrailMat = MaterialManager::getSingleton().create(
            "EnemyTrailMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        enemyTrailMat->setReceiveShadows(false);
        auto* te = enemyTrailMat->getTechnique(0);
        auto* pe = te->getPass(0);
        pe->setDiffuse(ColourValue(1.0f, 0.5f, 0.0f, 0.8f));
        pe->setAmbient(ColourValue(0.4f, 0.2f, 0.0f));
        pe->setSelfIllumination(ColourValue(0.8f, 0.4f, 0.0f));
        pe->setSceneBlending(SBT_TRANSPARENT_ALPHA);
        pe->setDepthWriteEnabled(false);
        pe->setLightingEnabled(true);
    }

    // ==================== ВСПОМОГАТЕЛЬНЫЕ МАТЕРИАЛЫ ==============
    MaterialPtr makeColorMaterial(const std::string& name, const ColourValue& color)
    {
        MaterialPtr mat = MaterialManager::getSingleton().getByName(name);
        if (!mat.isNull()) return mat;

        mat = MaterialManager::getSingleton().create(
            name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        auto* t = mat->getTechnique(0);
        auto* p = t->getPass(0);
        p->setDiffuse(color);
        p->setAmbient(color * 0.5f);
        p->setSelfIllumination(color * 0.4f);
        p->setLightingEnabled(true);
        return mat;
    }

    // ==================== ЗАГРУЗКА STL =====================
    Entity* createBikeModelFromStl(const std::string& name, const ColourValue& color)
    {
        const std::string filename = "STP_bike.stl"; // <-- правильное имя
        std::ifstream in(filename, std::ios::binary);
        if (!in)
        {
            std::cerr << "[STL] Не удалось открыть " << filename
                      << ". Используем простой бокс.\n";
            return createBikeModel(name + "_Box", color);
        }

        // Определяем формат: бинарный или ASCII
        in.seekg(0, std::ios::end);
        std::streamoff fsize = in.tellg();
        in.seekg(0, std::ios::beg);

        bool looksBinary = false;
        uint32_t triCountHeader = 0;
        if (fsize >= 84)
        {
            in.seekg(80, std::ios::beg);
            in.read(reinterpret_cast<char*>(&triCountHeader), 4);
            std::streamoff expected = 84 + static_cast<std::streamoff>(triCountHeader) * 50;
            if (expected == fsize && triCountHeader > 0) looksBinary = true;
        }
        in.seekg(0, std::ios::beg);

        ManualObject* mo = mSceneManager->createManualObject(name + "_STL");
        mo->begin("DefaultWhite", RenderOperation::OT_TRIANGLE_LIST);

        auto putTriangle = [&](const Vector3& n,
                               const Vector3& v0,
                               const Vector3& v1,
                               const Vector3& v2,
                               float scale,
                               bool zUpToYUp)
        {
            auto xform = [&](const Vector3& v)->Vector3
            {
                Vector3 r = v;
                if (zUpToYUp)
                {
                    // (x, y, z) -> (x, z, -y)
                    r = Vector3(v.x, v.z, -v.y);
                }
                return r * scale;
            };
            auto xformN = [&](const Vector3& v)->Vector3
            {
                Vector3 r = v;
                if (zUpToYUp)
                {
                    r = Vector3(v.x, v.z, -v.y);
                }
                r.normalise();
                return r;
            };

            Vector3 nn = xformN(n);
            Vector3 a  = xform(v0);
            Vector3 b  = xform(v1);
            Vector3 c  = xform(v2);

            size_t base = mo->getCurrentVertexCount();
            mo->position(a); mo->normal(nn);
            mo->position(b); mo->normal(nn);
            mo->position(c); mo->normal(nn);
            mo->triangle(base + 0, base + 1, base + 2);
        };

        int trianglesParsed = 0;
        const float stlScale = 0.05f; // подгон
        const bool  zUp      = true;  // большинство CAD-моделей в Z-up

        if (looksBinary)
        {
            // ---- Binary STL ----
            char header[80];
            in.read(header, 80);
            uint32_t triCount = 0;
            in.read(reinterpret_cast<char*>(&triCount), 4);

            for (uint32_t i = 0; i < triCount && in; ++i)
            {
                float nx, ny, nz;
                float vx[9];
                uint16_t attr;
                in.read(reinterpret_cast<char*>(&nx), 4);
                in.read(reinterpret_cast<char*>(&ny), 4);
                in.read(reinterpret_cast<char*>(&nz), 4);
                in.read(reinterpret_cast<char*>(vx), 36);
                in.read(reinterpret_cast<char*>(&attr), 2);

                Vector3 n(nx, ny, nz);
                Vector3 v0(vx[0], vx[1], vx[2]);
                Vector3 v1(vx[3], vx[4], vx[5]);
                Vector3 v2(vx[6], vx[7], vx[8]);

                putTriangle(n, v0, v1, v2, stlScale, zUp);
                ++trianglesParsed;
            }
        }
        else
        {
            // ---- ASCII STL ----
            std::string line;
            Vector3 n(0, 0, 1);
            while (std::getline(in, line))
            {
                std::istringstream ss(line);
                std::string word;
                ss >> word;
                if (word == "facet")
                {
                    std::string normalWord;
                    ss >> normalWord; // "normal"
                    float nx, ny, nz;
                    ss >> nx >> ny >> nz;
                    n = Vector3(nx, ny, nz);

                    std::getline(in, line); // outer loop
                    Vector3 v[3];
                    for (int i = 0; i < 3; ++i)
                    {
                        std::getline(in, line);
                        std::istringstream vs(line);
                        std::string vword;
                        float x, y, z;
                        vs >> vword >> x >> y >> z;
                        v[i] = Vector3(x, y, z);
                    }
                    std::getline(in, line); // endloop
                    std::getline(in, line); // endfacet

                    putTriangle(n, v[0], v[1], v[2], stlScale, zUp);
                    ++trianglesParsed;
                }
            }
        }

        mo->end();

        if (trianglesParsed == 0)
        {
            std::cerr << "[STL] Файл " << filename
                      << " загружен, но треугольников не найдено. Используем простой бокс.\n";
            mSceneManager->destroyManualObject(mo);
            return createBikeModel(name + "_Box", color);
        }

        std::cout << "[STL] Успешно загружен " << filename
                  << ", треугольников: " << trianglesParsed << std::endl;

        std::string matName = name + "_StlMat";
        MaterialPtr mat = makeColorMaterial(matName, color);
        mo->setMaterialName(0, matName);

        std::string meshName = name + "_StlMesh";
        MeshPtr mesh = mo->convertToMesh(meshName);
        mSceneManager->destroyManualObject(mo);

        Entity* ent = mSceneManager->createEntity(name + "_Entity", meshName);
        ent->setMaterialName(matName);
        return ent;
    }

    // ==================== МОДЕЛЬ МОТОЦИКЛА (БОКС) =====================
    Entity* createBikeModel(const std::string& name, const ColourValue& color)
    {
        ManualObject* m = mSceneManager->createManualObject(name);
        m->begin("DefaultWhite", RenderOperation::OT_TRIANGLE_LIST);

        float length = 4.0f;
        float width  = 1.2f;
        float height = 1.0f;

        Vector3 p[8];
        p[0] = Vector3(-length*0.5f, 0.0f, -width*0.5f);
        p[1] = Vector3(-length*0.5f, 0.0f,  width*0.5f);
        p[2] = Vector3(-length*0.5f, height, width*0.5f);
        p[3] = Vector3(-length*0.5f, height,-width*0.5f);

        p[4] = Vector3( length*0.5f, 0.0f, -width*0.5f);
        p[5] = Vector3( length*0.5f, 0.0f,  width*0.5f);
        p[6] = Vector3( length*0.5f, height, width*0.5f);
        p[7] = Vector3( length*0.5f, height,-width*0.5f);

        auto addQuad = [&](int a, int b, int c, int d)
        {
            Vector3 normal = (p[b] - p[a]).crossProduct(p[c] - p[a]);
            normal.normalise();

            size_t base = m->getCurrentVertexCount();
            m->position(p[a]); m->normal(normal);
            m->position(p[b]); m->normal(normal);
            m->position(p[c]); m->normal(normal);
            m->position(p[d]); m->normal(normal);

            m->triangle(base + 0, base + 1, base + 2);
            m->triangle(base + 0, base + 2, base + 3);
        };

        // Основной корпус
        addQuad(0, 1, 2, 3);
        addQuad(4, 7, 6, 5);
        addQuad(0, 3, 7, 4);
        addQuad(1, 5, 6, 2);
        addQuad(0, 4, 5, 1);
        addQuad(3, 2, 6, 7);

        // "сиденье"
        float seatH = height + 0.3f;
        Vector3 s[4];
        s[0] = Vector3(-length*0.2f, seatH, -width*0.5f);
        s[1] = Vector3(-length*0.2f, seatH,  width*0.5f);
        s[2] = Vector3( length*0.2f, seatH,  width*0.5f);
        s[3] = Vector3( length*0.2f, seatH, -width*0.5f);

        Vector3 nSeat(0, 1, 0);
        size_t base = m->getCurrentVertexCount();
        m->position(s[0]); m->normal(nSeat);
        m->position(s[1]); m->normal(nSeat);
        m->position(s[2]); m->normal(nSeat);
        m->position(s[3]); m->normal(nSeat);
        m->triangle(base + 0, base + 1, base + 2);
        m->triangle(base + 0, base + 2, base + 3);

        m->end();

        std::string matName = name + "_Mat";
        MaterialPtr mat = makeColorMaterial(matName, color);
        m->setMaterialName(0, matName);

        std::string meshName = name + "_Mesh";
        MeshPtr mesh = m->convertToMesh(meshName);
        mSceneManager->destroyManualObject(m);

        Entity* ent = mSceneManager->createEntity(name + "_Entity", meshName);
        ent->setMaterialName(matName);
        return ent;
    }

    // ======================= ИГРОК / ВРАГИ ======================
    void createPlayer()
    {
        // Пытаемся использовать STP_bike.stl, при проблемах — бокс
        mPlayerEntity = createBikeModelFromStl("PlayerBike", ColourValue(0.0f, 0.8f, 1.0f));

        mPlayerNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
        mPlayerNode->attachObject(mPlayerEntity);

        AxisAlignedBox bb = mPlayerEntity->getBoundingBox();
        float raise = -bb.getMinimum().y;
        mPlayerNode->translate(0, raise, 0);

        // случайная стартовая позиция около центра
        float sx = (std::rand() % (mGridSize / 4)) - (mGridSize / 8);
        float sz = (std::rand() % (mGridSize / 4)) - (mGridSize / 8);
        mPlayerNode->setPosition(sx * mCellSize, 0, sz * mCellSize);

        mPlayerYaw = Math::RangeRandom(0.0f, Math::TWO_PI);
        Quaternion qYaw(Radian(mPlayerYaw), Vector3::UNIT_Y);
        mPlayerNode->setOrientation(qYaw);

        std::cout << "Игрок создан.\n";
    }

    void createEnemies(int count)
    {
        for (int i = 0; i < count; ++i)
        {
            Enemy e{};
            e.entity = createBikeModelFromStl("EnemyBike_" + std::to_string(i),
                                              ColourValue(1.0f, 0.5f, 0.0f));
            e.node = mSceneManager->getRootSceneNode()->createChildSceneNode();
            e.node->attachObject(e.entity);

            AxisAlignedBox bb = e.entity->getBoundingBox();
            float raise = -bb.getMinimum().y;
            e.node->translate(0, raise, 0);

            Vector3 playerPos = mPlayerNode->getPosition();
            Vector3 pos;
            do {
                float sx = (std::rand() % mGridSize) - mGridSize * 0.5f;
                float sz = (std::rand() % mGridSize) - mGridSize * 0.5f;
                pos = Vector3(sx * mCellSize, 0, sz * mCellSize);
            } while ((pos - playerPos).length() < mMapHalfSize * 0.3f);
            e.node->setPosition(pos);

            e.direction = Vector3(
                (std::rand() % 100 - 50) / 50.0f,
                0.0f,
                (std::rand() % 100 - 50) / 50.0f).normalisedCopy();
            e.velocity = Vector3::ZERO;

            e.maxSpeed     = 20.0f + (std::rand() % 100) / 100.0f * 10.0f;
            e.acceleration = 25.0f + (std::rand() % 100) / 100.0f * 10.0f;
            e.deceleration = e.acceleration;
            e.directionChangeTimer    = 0.0f;
            e.directionChangeInterval = 1.5f + (std::rand() % 100) / 100.0f * 2.0f;

            e.yaw = std::atan2(e.direction.x, e.direction.z);
            e.node->setOrientation(Quaternion(Radian(e.yaw), Vector3::UNIT_Y));

            e.trailLastUpdate = 0.0f;
            e.lastTrailPos    = Vector3::ZERO;

            mEnemies.push_back(e);
        }

        std::cout << "Создано врагов: " << count << "\n";
    }

    // ================== СЛЕДЫ / СВЕТОВЫЕ СТЕНЫ ==================
    void clearTrail(std::deque<TrailSegment>& trail)
    {
        for (auto &seg : trail)
        {
            if (seg.node)
            {
                seg.node->detachAllObjects();
                mSceneManager->destroySceneNode(seg.node);
            }
            if (seg.object)
                mSceneManager->destroyManualObject(seg.object);
        }
        trail.clear();
    }

    void createTrailSegment(std::deque<TrailSegment>& trail,
                            const Vector3& position,
                            Vector3& lastPos,
                            bool isPlayerTrail,
                            float currentTime)
    {
        if (lastPos == Vector3::ZERO)
        {
            lastPos = position;
            return;
        }

        float dist = (position - lastPos).length();
        if (dist < 0.5f) return;

        TrailSegment seg{};
        seg.position     = position;
        seg.prevPosition = lastPos;
        seg.timestamp    = currentTime;
        seg.isPlayerTrail= isPlayerTrail;

        std::ostringstream name;
        name << (isPlayerTrail ? "player_trail_" : "enemy_trail_") << mTrailSegmentCounter++;

        seg.object = mSceneManager->createManualObject(name.str());
        seg.object->begin(isPlayerTrail ? "PlayerTrailMaterial" : "EnemyTrailMaterial",
                          RenderOperation::OT_TRIANGLE_LIST);

        Vector3 dir = position - lastPos;
        dir.y = 0;
        if (dir.squaredLength() < 1e-6f)
        {
            lastPos = position;
            seg.object->end();
            mSceneManager->destroyManualObject(seg.object);
            return;
        }
        dir.normalise();

        Vector3 perp(-dir.z, 0, dir.x);
        perp.normalise();
        float halfW = mTrailWidth * 0.5f;

        Vector3 p1 = lastPos  - perp * halfW;
        Vector3 p2 = lastPos  + perp * halfW;
        Vector3 p3 = position + perp * halfW;
        Vector3 p4 = position - perp * halfW;

        Vector3 up(0, mTrailHeight, 0);
        Vector3 p5 = p1 + up;
        Vector3 p6 = p2 + up;
        Vector3 p7 = p3 + up;
        Vector3 p8 = p4 + up;

        auto putQuad = [&](const Vector3& a,
                           const Vector3& b,
                           const Vector3& c,
                           const Vector3& d)
        {
            Vector3 n = (b - a).crossProduct(c - a);
            n.normalise();
            size_t base = seg.object->getCurrentVertexCount();
            seg.object->position(a); seg.object->normal(n);
            seg.object->position(b); seg.object->normal(n);
            seg.object->position(c); seg.object->normal(n);
            seg.object->position(d); seg.object->normal(n);
            seg.object->triangle(base+0, base+1, base+2);
            seg.object->triangle(base+0, base+2, base+3);
        };

        // 6 граней
        putQuad(p1, p2, p3, p4); // низ
        putQuad(p5, p8, p7, p6); // верх
        putQuad(p1, p5, p6, p2); // бок
        putQuad(p4, p3, p7, p8); // другой бок
        putQuad(p1, p4, p8, p5); // торец
        putQuad(p2, p6, p7, p3); // торец

        seg.object->end();

        seg.node = mSceneManager->getRootSceneNode()->createChildSceneNode();
        seg.node->attachObject(seg.object);

        trail.push_back(seg);
        lastPos = position;
    }

    void updateTrails(float dt, float currentTime)
    {
        // игрок
        if (std::fabs(mCurrentSpeed) > 0.5f)
        {
            mTrailLastUpdate += dt;
            if (mTrailLastUpdate >= mTrailUpdateInterval)
            {
                Vector3 p = mPlayerNode->getPosition();
                createTrailSegment(mPlayerTrail, p, mPlayerLastTrailPos, true, currentTime);
                mTrailLastUpdate = 0.0f;
            }
        }

        while (!mPlayerTrail.empty() &&
               (currentTime - mPlayerTrail.front().timestamp) > mTrailLifetime)
        {
            TrailSegment &old = mPlayerTrail.front();
            if (old.node)
            {
                old.node->detachAllObjects();
                mSceneManager->destroySceneNode(old.node);
            }
            if (old.object)
                mSceneManager->destroyManualObject(old.object);
            mPlayerTrail.pop_front();
        }

        // враги
        for (auto &e : mEnemies)
        {
            if (e.velocity.length() > 0.5f)
            {
                e.trailLastUpdate += dt;
                if (e.trailLastUpdate >= mTrailUpdateInterval)
                {
                    Vector3 p = e.node->getPosition();
                    createTrailSegment(e.trail, p, e.lastTrailPos, false, currentTime);
                    e.trailLastUpdate = 0.0f;
                }
            }

            while (!e.trail.empty() &&
                   (currentTime - e.trail.front().timestamp) > mTrailLifetime)
            {
                TrailSegment &old = e.trail.front();
                if (old.node)
                {
                    old.node->detachAllObjects();
                    mSceneManager->destroySceneNode(old.node);
                }
                if (old.object)
                    mSceneManager->destroyManualObject(old.object);
                e.trail.pop_front();
            }
        }
    }

    // ====================== ФИЗИКА ИГРОКА =======================
    void updatePlayerMovement(float dt)
    {
        // --- продольная скорость ---
        float acc = 0.0f;

        if (mForward)  acc += mAcceleration;
        if (mBackward)
        {
            if (mCurrentSpeed > 0.0f) acc -= mBrakeDecel; // активный тормоз
            else                      acc -= mAcceleration;
        }

        if (mForward || mBackward) mCurrentSpeed += acc * dt;
        else
        {
            // трение
            if (mCurrentSpeed > 0.0f)
            { mCurrentSpeed -= mFriction * dt; if (mCurrentSpeed < 0.0f) mCurrentSpeed = 0.0f; }
            else if (mCurrentSpeed < 0.0f)
            { mCurrentSpeed += mFriction * dt; if (mCurrentSpeed > 0.0f) mCurrentSpeed = 0.0f; }
        }

        // ограничения
        if (mCurrentSpeed >  mMaxForwardSpeed)  mCurrentSpeed =  mMaxForwardSpeed;
        if (mCurrentSpeed < -mMaxBackwardSpeed) mCurrentSpeed = -mMaxBackwardSpeed;

        // --- поворот и наклон ---
        float turnInput = 0.0f;
        if (mLeft)  turnInput += 1.0f;
        if (mRight) turnInput -= 1.0f;

        float dirSign = (mCurrentSpeed >= 0.0f ? 1.0f : -1.0f);
        mPlayerYaw += turnInput * mTurnSpeed * dt * dirSign;

        float speedFactor = std::min(1.0f, std::fabs(mCurrentSpeed) / mMaxForwardSpeed);
        float targetLean = turnInput * mMaxLeanAngle * speedFactor;

        float lerpFactor = std::min(1.0f, mLeanSpeed * dt);
        mCurrentLean += (targetLean - mCurrentLean) * lerpFactor;

        Quaternion yawQ(Radian(mPlayerYaw), Vector3::UNIT_Y);
        Quaternion rollQ(Radian(mCurrentLean), Vector3::UNIT_Z);
        mPlayerNode->setOrientation(yawQ * rollQ);

        Vector3 forward = yawQ * Vector3(0, 0, -1);
        Vector3 delta = forward * (mCurrentSpeed * dt);
        Vector3 newPos = mPlayerNode->getPosition() + delta;

        float border = mMapHalfSize - 2.0f;
        if (newPos.x < -border) newPos.x = -border;
        if (newPos.x >  border) newPos.x =  border;
        if (newPos.z < -border) newPos.z = -border;
        if (newPos.z >  border) newPos.z =  border;

        mPlayerNode->setPosition(newPos);
    }

    // ======================= ЛОГИКА ВРАГОВ ======================
    void updateEnemies(float dt)
    {
        for (auto &e : mEnemies)
        {
            e.directionChangeTimer += dt;
            if (e.directionChangeTimer >= e.directionChangeInterval)
            {
                e.direction = Vector3(
                    (std::rand() % 100 - 50) / 50.0f,
                    0.0f,
                    (std::rand() % 100 - 50) / 50.0f).normalisedCopy();
                e.directionChangeTimer = 0.0f;
                e.directionChangeInterval = 1.5f + (std::rand() % 100) / 100.0f * 2.0f;
            }

            Vector3 desiredVel = e.direction * e.maxSpeed;
            Vector3 accel = (desiredVel - e.velocity) * e.acceleration * dt;
            e.velocity += accel;

            if (e.velocity.length() > e.maxSpeed)
            {
                e.velocity.normalise();
                e.velocity *= e.maxSpeed;
            }

            if (e.velocity.length() > 0.1f)
                e.yaw = std::atan2(e.velocity.x, e.velocity.z);

            Vector3 newPos = e.node->getPosition() + e.velocity * dt;
            float border = mMapHalfSize - 2.0f;
            if (newPos.x < -border || newPos.x > border)
            {
                e.direction.x = -e.direction.x;
                e.velocity.x  = -e.velocity.x * 0.8f;
                newPos.x = std::max(-border, std::min(border, newPos.x));
            }
            if (newPos.z < -border || newPos.z > border)
            {
                e.direction.z = -e.direction.z;
                e.velocity.z  = -e.velocity.z * 0.8f;
                newPos.z = std::max(-border, std::min(border, newPos.z));
            }

            e.node->setPosition(newPos);
            e.node->setOrientation(Quaternion(Radian(e.yaw), Vector3::UNIT_Y));
        }
    }

    // ======================== КАМЕРА =============================
    void updateCameraPosition()
    {
        if (!mPlayerNode) return;

        Vector3 playerPos = mPlayerNode->getPosition();

        Vector3 offset(0, 0, mCameraDistance);
        Quaternion pitchQ(Radian(mCameraPitch), Vector3::UNIT_X);
        Quaternion yawQ(Radian(mCameraYaw),   Vector3::UNIT_Y);
        Quaternion camQ = yawQ * pitchQ;
        offset = camQ * offset;

        Vector3 camPos = playerPos + offset;
        camPos.y += mCameraHeight;

        mCamera->setPosition(camPos);
        mCamera->lookAt(playerPos + Vector3(0, 2, 0));
    }
};

// ============================= main ==============================
int main(int, char**)
{
    try
    {
        SimpleApp app;
        app.initApp();
        app.getRoot()->startRendering();
        app.closeApp();
    }
    catch (const Ogre::Exception& e)
    {
        std::cerr << "OGRE ERROR: " << e.getFullDescription() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
