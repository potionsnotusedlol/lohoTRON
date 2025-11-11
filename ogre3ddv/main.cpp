#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>
#include <SDL2/SDL_keycode.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <deque>
#include <vector>
#include <limits>

using namespace Ogre;
using namespace OgreBites;

// ---------- утилиты ----------
static inline float clampf(float v, float a, float b) { return std::max(a, std::min(b, v)); }
static inline float lerpf(float a, float b, float t)  { return a + (b - a) * t; }
static inline float sqrf(float x){ return x*x; }

// ближайшие точки между двумя отрезками в XZ (2D)
struct Closest2D { float t; float u; float dist2; };
static Closest2D closestSegSeg2D(const Vector2& p0, const Vector2& p1, const Vector2& q0, const Vector2& q1)
{
    const float EPS = 1e-6f;
    Vector2   u = p1 - p0; // наш ход
    Vector2   v = q1 - q0; // сегмент следа/ход соперника
    Vector2   w = p0 - q0;
    float     a = u.dotProduct(u);
    float     b = u.dotProduct(v);
    float     c = v.dotProduct(v);
    float     d = u.dotProduct(w);
    float     e = v.dotProduct(w);
    float     D = a*c - b*b;

    float sn, sd = D;
    float tn, td = D;

    if (D < EPS) { sn = 0.0f; sd = 1.0f; tn = e; td = c; }
    else {
        sn = (b*e - c*d);
        tn = (a*e - b*d);
        if (sn < 0.0f) { sn = 0.0f; tn = e; td = c; }
        else if (sn > sd) { sn = sd; tn = e + b; td = c; }
    }

    if (tn < 0.0f) {
        tn = 0.0f;
        if (-d < 0.0f) sn = 0.0f;
        else if (-d > a) sn = sd;
        else { sn = -d; sd = a; }
    }
    else if (tn > td) {
        tn = td;
        if ((-d + b) < 0.0f) sn = 0.0f;
        else if ((-d + b) > a) sn = sd;
        else { sn = (-d + b); sd = a; }
    }

    float sc = (std::fabs(sn) < EPS ? 0.0f : sn / sd);
    float tc = (std::fabs(tn) < EPS ? 0.0f : tn / td);

    Vector2 dP = w + (u * sc) - (v * tc);
    return { sc, tc, dP.squaredLength() };
}

struct TrailPoint { Vector3 pos; float t; };

struct Player {
    // визуал
    SceneNode* node = nullptr;
    Entity*    ent  = nullptr;
    Light*     glow = nullptr;

    // движение
    float yaw = 0.0f;
    float speed = 0.0f;
    float lean  = 0.0f;

    // идентичность
    ColourValue color = ColourValue(0.2f, 0.7f, 1.0f);
    String      name;
    bool        human = false;
    bool        alive = true;

    // след
    BillboardChain* trail = nullptr;
    std::deque<TrailPoint> pts;    // точки следа (мировые), с таймштампом
    Vector3 lastTrailPos = Vector3::ZERO;

    // перемещение в кадре (для коллизий)
    Vector3 framePrevPos = Vector3::ZERO;
    Vector3 frameNewPos  = Vector3::ZERO;

    // ИИ
    float aiTurnTimer = 0.0f;
    float aiTurnDir   = 0.0f; // -1..+1
};

class SimpleApp :
        public ApplicationContext,
        public InputListener
{
public:
    SimpleApp()
        : ApplicationContext("TronBike_Cubes_Collision"),
          mSceneManager(nullptr), mWindow(nullptr), mCamera(nullptr),
          // TPS rig
          mCamPivot(nullptr), mCamYawNode(nullptr), mCamPitchNode(nullptr), mCamEye(nullptr),
          mCamYaw(0.0f), mCamPitch(-0.35f),
          mCamTargetHeight(2.0f), mCamDistance(6.0f), mCamDistCurrent(6.0f),
          mCamMinDist(2.0f), mCamMaxDist(40.0f),
          mCamSmooth(12.0f), mCamFollowYawSmooth(8.0f),
          mRmbDown(false), mMouseSensitivity(0.002f),
          // grid
          mGridSize(80), mCellSize(2.0f), mMapHalfSize((mGridSize*mCellSize)*0.5f),
          // input
          mForward(false), mBackward(false), mLeft(false), mRight(false),
          // движение
          mMaxForwardSpeed(40.0f), mMaxBackwardSpeed(20.0f),
          mAcceleration(45.0f), mBrakeDecel(70.0f), mFriction(25.0f), mTurnSpeed(2.8f),
          mMaxLeanAngle(Degree(35.0f).valueRadians()), mLeanSpeed(6.0f),
          // след
          mTrailTTL(10.0f), mTrailMinSegDist(0.5f), mTrailWidth(0.7f),
          // коллизии
          mPlayerRadius(0.6f), mSelfSkipSegments(2), mSelfTouchEps(1e-3f),
          // time/players
          mGameTime(0.0f), mHumanIndex(0)
    { std::srand(unsigned(std::time(nullptr))); }

    void setup() override
    {
        ApplicationContext::setup();
        addInputListener(this);

        mWindow = getRenderWindow();
        try { mWindow->resize(3840, 2160); } catch (...) {}

        Root* root = getRoot();
        mSceneManager = root->createSceneManager();
        if (!RTShader::ShaderGenerator::getSingletonPtr())
            RTShader::ShaderGenerator::initialize();
        auto* sg = RTShader::ShaderGenerator::getSingletonPtr();
        sg->addSceneManager(mSceneManager);
        MaterialManager::getSingleton().setActiveScheme(
            RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

        // камера
        mCamera = mSceneManager->createCamera("MainCamera");
        mCamera->setNearClipDistance(0.1f);
        mCamera->setFarClipDistance(2000.0f);
        mCamera->setAutoAspectRatio(true);
        Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(ColourValue(0.0f, 0.0f, 0.03f));

        // TPS rig
        mCamPivot     = mSceneManager->getRootSceneNode()->createChildSceneNode("CamPivot");
        mCamYawNode   = mCamPivot->createChildSceneNode("CamYaw");
        mCamPitchNode = mCamYawNode->createChildSceneNode("CamPitch");
        mCamEye       = mCamPitchNode->createChildSceneNode("CamEye");
        mCamEye->attachObject(mCamera);
        mCamYawNode->setOrientation(Quaternion(Radian(mCamYaw), Vector3::UNIT_Y));
        mCamPitchNode->setOrientation(Quaternion(Radian(mCamPitch), Vector3::UNIT_X));
        mCamEye->setPosition(0, 0, +mCamDistCurrent);

        // свет
        {
            Light* mainLight = mSceneManager->createLight("MainLight");
            mainLight->setType(Light::LT_DIRECTIONAL);
            SceneNode* lightNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
            lightNode->attachObject(mainLight);
            lightNode->setDirection(Vector3(-0.5f, -1.0f, -0.3f).normalisedCopy());
            mainLight->setDiffuseColour(ColourValue(0.7f, 0.7f, 0.7f));

            Light* neon = mSceneManager->createLight("Neon");
            neon->setType(Light::LT_POINT);
            SceneNode* n = mSceneManager->getRootSceneNode()->createChildSceneNode(Vector3(0, 50, 0));
            n->attachObject(neon);
            neon->setDiffuseColour(ColourValue(0.1f, 0.2f, 0.6f));
            neon->setSpecularColour(ColourValue(0.1f, 0.2f, 0.6f));
        }

        createGrid();

        // спавны и раунд
        mSpawns = { Vector3(0,0,0), Vector3(20,0,20), Vector3(-25,0,-15), Vector3(-15,0,30) };
        spawnAllPlayers();

        root->addFrameListener(this);

        std::cout << "ПКМ — вращать, колесо — зум. W/S — газ/тормоз, A/D — поворот, ESC — выход\n";
    }

    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        if (mWindow->isClosed()) return false;

        float dt = evt.timeSinceLastFrame;
        mGameTime += dt;

        // 1) стартовые позиции
        for (auto& P : mPlayers) if (P.alive) P.framePrevPos = P.node->getPosition();

        // 2) движение (без дорисовки следа)
        for (auto& P : mPlayers) {
            if (!P.alive) continue;
            if (P.human) updateMove(P, dt, mForward, mBackward, mLeft, mRight);
            else { updateAI(P, dt); updateMove(P, dt, true, false, P.aiTurnDir > 0, P.aiTurnDir < 0); }
            P.frameNewPos = P.node->getPosition();
        }

        // 3) коллизии (кто раньше коснулся — погиб)
        resolveCollisionsAndDeaths();

        // если умер игрок — рестарт
        if (!mPlayers[mHumanIndex].alive) { startNewRound(); return true; }

        // если остался 1 — рестарт
        if (aliveCount() <= 1) { startNewRound(); return true; }

        // 4) дорисовываем следы живым
        for (auto& P : mPlayers) if (P.alive) updateTrail(P);

        // 5) камера
        updateCameraRig(dt);

        return true;
    }

    // ---------- input ----------
    bool keyPressed(const KeyboardEvent& e) override {
        switch (e.keysym.sym) {
            case SDLK_w: mForward  = true; break;
            case SDLK_s: mBackward = true; break;
            case SDLK_a: mLeft     = true; break;
            case SDLK_d: mRight    = true; break;
            default: break;
        }
        return true;
    }
    bool keyReleased(const KeyboardEvent& e) override {
        switch (e.keysym.sym) {
            case SDLK_w: mForward  = false; break;
            case SDLK_s: mBackward = false; break;
            case SDLK_a: mLeft     = false; break;
            case SDLK_d: mRight    = false; break;
            default: break;
        }
        return true;
    }
    bool mousePressed(const MouseButtonEvent& e) override { if (e.button == BUTTON_RIGHT) mRmbDown = true; return true; }
    bool mouseReleased(const MouseButtonEvent& e) override { if (e.button == BUTTON_RIGHT) mRmbDown = false; return true; }
    bool mouseMoved(const MouseMotionEvent& e) override {
        if (mRmbDown) {
            mCamYaw   -= float(e.xrel) * mMouseSensitivity;
            mCamPitch -= float(e.yrel) * mMouseSensitivity;
            mCamPitch = clampf(mCamPitch, Degree(-85).valueRadians(), Degree(85).valueRadians());
            mCamYawNode->setOrientation(Quaternion(Radian(mCamYaw),   Vector3::UNIT_Y));
            mCamPitchNode->setOrientation(Quaternion(Radian(mCamPitch), Vector3::UNIT_X));
        }
        return true;
    }
    bool mouseWheelRolled(const MouseWheelEvent& e) override {
        mCamDistance = clampf(mCamDistance - float(e.y)*1.0f, mCamMinDist, mCamMaxDist);
        return true;
    }

private:
    // ogre
    SceneManager*  mSceneManager;
    RenderWindow*  mWindow;
    Camera*        mCamera;

    // TPS rig
    SceneNode* mCamPivot;  SceneNode* mCamYawNode;  SceneNode* mCamPitchNode; SceneNode* mCamEye;
    float mCamYaw, mCamPitch;
    float mCamTargetHeight;
    float mCamDistance, mCamDistCurrent;
    float mCamMinDist, mCamMaxDist;
    float mCamSmooth;
    float mCamFollowYawSmooth;
    bool  mRmbDown;
    float mMouseSensitivity;

    // grid
    int   mGridSize; float mCellSize; float mMapHalfSize;

    // input
    bool mForward, mBackward, mLeft, mRight;

    // движение
    float mMaxForwardSpeed, mMaxBackwardSpeed;
    float mAcceleration, mBrakeDecel, mFriction, mTurnSpeed;
    float mMaxLeanAngle, mLeanSpeed;

    // след
    float mTrailTTL, mTrailMinSegDist, mTrailWidth;

    // коллизии
    float mPlayerRadius;
    size_t mSelfSkipSegments; // сколько последних своих сегментов игнорим
    float  mSelfTouchEps;     // чтобы не ловить касание в начале хода

    // players
    std::vector<Player> mPlayers;
    size_t mHumanIndex;
    std::vector<Vector3> mSpawns;

    // time
    float mGameTime;

private:
    // ---------- мир ----------
    void createGrid()
    {
        // материалы
        {
            MaterialPtr gmat = MaterialManager::getSingleton().create(
                "GroundMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            Pass* p = gmat->getTechnique(0)->getPass(0);
            p->setLightingEnabled(true);
            p->setDiffuse(ColourValue(0.02f, 0.02f, 0.05f));
            p->setAmbient(ColourValue(0.01f, 0.01f, 0.02f));
            p->setSelfIllumination(ColourValue(0.02f, 0.02f, 0.05f));
        }
        {
            MaterialPtr lmat = MaterialManager::getSingleton().create(
                "GridMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            Pass* p = lmat->getTechnique(0)->getPass(0);
            p->setLightingEnabled(false);
            p->setDiffuse(ColourValue(0.0f, 0.55f, 1.0f));
            p->setSelfIllumination(ColourValue(0.0f, 0.45f, 1.0f));
            p->setSceneBlending(SBT_ADD);
            p->setDepthWriteEnabled(false);
        }

        // пол
        ManualObject* ground = mSceneManager->createManualObject("Ground");
        ground->begin("GroundMaterial", RenderOperation::OT_TRIANGLE_LIST);
        float half = mMapHalfSize;
        for (int x = 0; x < mGridSize; ++x)
        for (int z = 0; z < mGridSize; ++z) {
            float x0 = x * mCellSize - half, x1 = x0 + mCellSize;
            float z0 = z * mCellSize - half, z1 = z0 + mCellSize;
            Vector3 v0(x0,0,z0), v1(x1,0,z0), v2(x1,0,z1), v3(x0,0,z1);
            size_t base = ground->getCurrentVertexCount();
            ground->position(v0); ground->normal(Vector3::UNIT_Y);
            ground->position(v1); ground->normal(Vector3::UNIT_Y);
            ground->position(v2); ground->normal(Vector3::UNIT_Y);
            ground->position(v3); ground->normal(Vector3::UNIT_Y);
            ground->triangle(base+0,base+1,base+2);
            ground->triangle(base+0,base+2,base+3);
        }
        ground->end();
        mSceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(ground);

        // сетка
        ManualObject* grid = mSceneManager->createManualObject("GridLines");
        grid->begin("GridMaterial", RenderOperation::OT_LINE_LIST);
        for (int i = 0; i <= mGridSize; ++i) {
            float p = i * mCellSize - half;
            grid->position(-half, 0.02f, p); grid->position(+half, 0.02f, p);
            grid->position(p, 0.02f, -half); grid->position(p, 0.02f, +half);
        }
        grid->end();
        mSceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(grid);
    }

    Entity* createBoxEntity(const String& meshName, const String& entName,
                            float L=2.5f, float W=1.6f, float H=1.6f,
                            const ColourValue& color=ColourValue(0.8f,0.8f,1.0f))
    {
        String matName = entName + "_Mat";
        {
            MaterialPtr mat = MaterialManager::getSingleton().create(
                matName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            Pass* p = mat->getTechnique(0)->getPass(0);
            p->setLightingEnabled(true);
            p->setDiffuse(color);
            p->setAmbient(color * 0.35f);
            p->setSelfIllumination(color * 0.55f);
        }

        ManualObject* m = mSceneManager->createManualObject(entName + "_MO");
        m->begin(matName, RenderOperation::OT_TRIANGLE_LIST);

        Vector3 p[8] = {
            {-L*0.5f, 0.0f, -W*0.5f}, {-L*0.5f, 0.0f,  W*0.5f},
            {-L*0.5f, H,     W*0.5f}, {-L*0.5f, H,    -W*0.5f},
            { L*0.5f, 0.0f, -W*0.5f}, { L*0.5f, 0.0f,  W*0.5f},
            { L*0.5f, H,     W*0.5f}, { L*0.5f, H,    -W*0.5f}
        };
        auto addQuad=[&](int a,int b,int c,int d){
            Vector3 n=(p[b]-p[a]).crossProduct(p[c]-p[a]); n.normalise();
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

        MeshPtr mesh = m->convertToMesh(meshName);
        mSceneManager->destroyManualObject(m);

        Entity* ent = mSceneManager->createEntity(entName, meshName);
        ent->setMaterialName(matName);
        return ent;
    }

    void spawnPlayer(bool human, const Vector3& start, const ColourValue& color, const String& name)
    {
        Player P;
        P.human = human;
        P.color = ColourValue(color.r, color.g, color.b, 1.0f);
        P.name  = name;
        P.alive = true;

        P.ent  = createBoxEntity(name+"_BoxMesh", name+"_BoxEntity", 2.5f, 1.6f, 1.6f, P.color);
        P.node = mSceneManager->getRootSceneNode()->createChildSceneNode(name + "_Node");
        P.node->attachObject(P.ent);

        AxisAlignedBox bb = P.ent->getBoundingBox();
        float raise = -bb.getMinimum().y;
        P.node->translate(0, raise, 0);

        P.node->setPosition(start);
        P.yaw = 0.0f; P.speed = 0.0f; P.lean = 0.0f;
        P.node->setOrientation(Quaternion(Radian(P.yaw), Vector3::UNIT_Y));

        // свет
        P.glow = mSceneManager->createLight(name + "_Glow");
        P.glow->setType(Light::LT_POINT);
        P.glow->setDiffuseColour(P.color * 1.5f);
        P.glow->setSpecularColour(P.color);
        P.glow->setAttenuation(120.0f, 1.0f, 0.005f, 0.0005f);
        P.node->createChildSceneNode(name+"_GlowNode", Vector3(0, 1.0f, 0))->attachObject(P.glow);

        // материал следа
        {
            MaterialPtr m = MaterialManager::getSingleton().create(
                name + "_TrailMat", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            Pass* ps = m->getTechnique(0)->getPass(0);
            ps->setLightingEnabled(false);
            ps->setDiffuse(P.color);
            ps->setAmbient(ColourValue::Black);
            ps->setSelfIllumination(P.color);
            ps->setSceneBlending(SBT_ADD);
            ps->setDepthWriteEnabled(false);
        }

        // цепочка
        P.trail = mSceneManager->createBillboardChain(name + "_Trail");
        P.trail->setNumberOfChains(1);
        P.trail->setMaxChainElements(1024);
        P.trail->setUseTextureCoords(false);
        P.trail->setUseVertexColours(true);
        P.trail->setMaterialName(name + "_TrailMat");
        mSceneManager->getRootSceneNode()->createChildSceneNode(name + "_TrailNode")->attachObject(P.trail);

        // ИИ
        if (!P.human) { P.aiTurnTimer = frand(0.5f, 2.0f); P.aiTurnDir = (frand(0.0f,1.0f)>0.5f?+1.0f:-1.0f); }

        P.lastTrailPos = start;
        P.framePrevPos = start;
        P.frameNewPos  = start;

        mPlayers.push_back(P);
    }

    void spawnAllPlayers()
    {
        mPlayers.clear();
        spawnPlayer(true,  mSpawns[0], ColourValue(0.2f, 0.7f, 1.0f), "Player");
        spawnPlayer(false, mSpawns[1], ColourValue(1.0f, 0.3f, 0.3f), "Bot_Red");
        spawnPlayer(false, mSpawns[2], ColourValue(0.2f, 1.0f, 0.4f), "Bot_Green");
        spawnPlayer(false, mSpawns[3], ColourValue(1.0f, 0.8f, 0.2f), "Bot_Yellow");
        mHumanIndex = 0;
    }

    static float frand(float a, float b) { return a + float(std::rand())/float(RAND_MAX) * (b - a); }

    // ---------- движение ----------
    void updateMove(Player& P, float dt, bool W, bool S, bool A, bool D)
    {
        float acc = 0.0f;
        if (W) acc += mAcceleration;
        if (S) acc += (P.speed > 0.0f ? -mBrakeDecel : -mAcceleration);

        if (W || S) {
            P.speed += acc * dt;
        } else {
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

        Quaternion yawQ(Radian(P.yaw), Vector3::UNIT_Y);
        Quaternion rollQ(Radian(P.lean), Vector3::UNIT_Z);
        P.node->setOrientation(yawQ * rollQ);

        Vector3 forward = yawQ * Vector3(0, 0, -1);
        Vector3 newPos  = P.node->getPosition() + forward * (P.speed * dt);

        float border = mMapHalfSize - 2.0f;
        newPos.x = clampf(newPos.x, -border, border);
        newPos.z = clampf(newPos.z, -border, border);
        P.node->setPosition(newPos);
    }

    void updateAI(Player& P, float dt)
    {
        P.aiTurnTimer -= dt;
        if (P.aiTurnTimer <= 0.0f) {
            P.aiTurnTimer = frand(0.7f, 2.0f);
            P.aiTurnDir   = (frand(0.0f, 1.0f) > 0.5f ? +1.0f : -1.0f);
        }
        Vector3 pos = P.node->getPosition();
        float edge = mMapHalfSize * 0.85f;
        if (std::fabs(pos.x) > edge || std::fabs(pos.z) > edge) {
            Vector3 toCenter = -pos;
            Vector3 fwd = (Quaternion(Radian(P.yaw), Vector3::UNIT_Y) * Vector3(0,0,-1));
            float side = fwd.crossProduct(toCenter).y;
            P.aiTurnDir = (side > 0 ? +1.0f : -1.0f);
            P.aiTurnTimer = 0.3f;
        }
    }

    // ---------- след ----------
    void updateTrail(Player& P)
    {
        const Vector3 pos = P.node->getPosition() + Vector3(0, 0.2f, 0);
        if (P.pts.empty() || (pos - P.lastTrailPos).length() >= mTrailMinSegDist) {
            P.pts.push_back({pos, mGameTime});
            P.lastTrailPos = pos;
        }

        // чистим старьё
        while (!P.pts.empty() && (mGameTime - P.pts.front().t) > mTrailTTL)
            P.pts.pop_front();

        // рисуем цепочку
        P.trail->clearChain(0);
        for (const auto& tp : P.pts) {
            float age = mGameTime - tp.t;
            float a   = clampf(1.0f - age / mTrailTTL, 0.0f, 1.0f);
            ColourValue c = P.color; c.a = a;
            BillboardChain::Element el(tp.pos, mTrailWidth, 0.0f, c, Quaternion::IDENTITY);
            P.trail->addChainElement(0, el);
        }
    }

    // ---------- столкновения ----------
    struct Hit { size_t idx; float alpha; }; // игрок и "время" касания 0..1 вдоль его хода

    void resolveCollisionsAndDeaths()
    {
        const float trailRad = mTrailWidth * 0.5f;
        const float epsAlpha = 1e-4f;

        std::vector<Hit> hits;

        for (size_t i = 0; i < mPlayers.size(); ++i) {
            Player& A = mPlayers[i];
            if (!A.alive) continue;

            Vector2 a0(A.framePrevPos.x, A.framePrevPos.z);
            Vector2 a1(A.frameNewPos.x,  A.frameNewPos.z);

            float bestAlpha = std::numeric_limits<float>::infinity();
            bool  any = false;

            // A) против следов всех игроков
            for (size_t j = 0; j < mPlayers.size(); ++j) {
                const Player& B = mPlayers[j];
                size_t segCount = (B.pts.size() >= 2) ? (B.pts.size() - 1) : 0;
                if (segCount == 0) continue;

                // если проверяем свой след — пропустим последние mSelfSkipSegments
                size_t limit = segCount;
                if (j == i) {
                    if (limit > mSelfSkipSegments) limit -= mSelfSkipSegments;
                    else limit = 0;
                }

                for (size_t k = 0; k < limit; ++k) {
                    Vector2 b0(B.pts[k].pos.x,   B.pts[k].pos.z);
                    Vector2 b1(B.pts[k+1].pos.x, B.pts[k+1].pos.z);

                    Closest2D cl = closestSegSeg2D(a0, a1, b0, b1);

                    // игнорируем касание в самом начале своего хода (стык со своей лентой)
                    if (j == i && cl.t <= mSelfTouchEps) continue;

                    float r = mPlayerRadius + trailRad;
                    if (cl.dist2 <= r*r) {
                        any = true;
                        if (cl.t < bestAlpha) bestAlpha = cl.t;
                    }
                }
            }

            // B) против перемещений других игроков (столкновение тел)
            for (size_t j = 0; j < mPlayers.size(); ++j) if (j != i) {
                const Player& B = mPlayers[j];
                if (!B.alive) continue;
                Vector2 b0(B.framePrevPos.x, B.framePrevPos.z);
                Vector2 b1(B.frameNewPos.x,  B.frameNewPos.z);
                Closest2D cl = closestSegSeg2D(a0, a1, b0, b1);
                float r = mPlayerRadius + mPlayerRadius;
                if (cl.dist2 <= r*r) {
                    any = true;
                    if (cl.t < bestAlpha) bestAlpha = cl.t;
                }
            }

            if (any) hits.push_back({ i, bestAlpha });
        }

        if (hits.empty()) return;

        // минимальный alpha — кто раньше коснулся
        float best = hits[0].alpha;
        for (auto& h : hits) best = std::min(best, h.alpha);

        for (auto& h : hits) {
            if (std::fabs(h.alpha - best) <= epsAlpha) killPlayer(h.idx);
        }
    }

    void killPlayer(size_t idx)
    {
        Player& P = mPlayers[idx];
        if (!P.alive) return;
        P.alive = false;
        if (P.node) P.node->setVisible(false, true); // тело скрываем, след доживает TTL
    }

    size_t aliveCount() const
    {
        size_t n = 0;
        for (auto& P : const_cast<std::vector<Player>&>(mPlayers)) if (P.alive) ++n;
        return n;
    }

    void startNewRound()
    {
        for (size_t i = 0; i < mPlayers.size(); ++i) {
            Player& P = mPlayers[i];
            P.alive = true; P.speed = 0.0f; P.yaw = 0.0f; P.lean = 0.0f;

            Vector3 start = mSpawns[std::min(i, mSpawns.size()-1)];
            P.node->setPosition(start);
            P.node->setOrientation(Quaternion(Radian(P.yaw), Vector3::UNIT_Y));
            P.node->setVisible(true, true);

            P.framePrevPos = start;
            P.frameNewPos  = start;

            P.pts.clear(); P.trail->clearChain(0); P.lastTrailPos = start;

            if (!P.human) { P.aiTurnTimer = frand(0.5f, 2.0f); P.aiTurnDir = (frand(0.0f,1.0f)>0.5f?+1.0f:-1.0f); }
        }
        // мGameTime не сбрасываем — TTL по абсолютному времени ок, следы очищены
    }

    // ---------- камера ----------
    static float wrapPi(float a) {
        while (a >  Math::PI) a -= Math::TWO_PI;
        while (a < -Math::PI) a += Math::TWO_PI;
        return a;
    }

    void updateCameraRig(float dt)
    {
        if (mPlayers.empty()) return;
        Player& H = mPlayers[mHumanIndex];

        Vector3 target = H.node->getPosition() + Vector3(0, mCamTargetHeight, 0);
        mCamPivot->setPosition(target);

        if (!mRmbDown) {
            float diff = wrapPi(H.yaw - mCamYaw);
            float t = 1.0f - std::exp(-mCamFollowYawSmooth * dt);
            mCamYaw += diff * t;
            mCamYawNode->setOrientation(Quaternion(Radian(mCamYaw), Vector3::UNIT_Y));
        }

        float tz = 1.0f - std::exp(-mCamSmooth * dt);
        mCamDistCurrent += (mCamDistance - mCamDistCurrent) * tz;

        mCamPitchNode->setOrientation(Quaternion(Radian(mCamPitch), Vector3::UNIT_X));
        mCamEye->setPosition(0, 0, +mCamDistCurrent);
    }
};

int main(int, char**)
{
    try {
        SimpleApp app;
        app.initApp();
        app.getRoot()->startRendering();
        app.closeApp();
    }
    catch (const Ogre::Exception& e) {
        std::cerr << "OGRE ERROR: " << e.getFullDescription() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
