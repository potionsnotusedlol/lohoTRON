#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>
#include <SDL2/SDL_keycode.h>

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace Ogre;
using namespace OgreBites;

class SimpleApp :
        public ApplicationContext,
        public InputListener
{
public:
    SimpleApp()
        : ApplicationContext("TronBikeDemo_TPS"),
          mSceneManager(nullptr),
          mWindow(nullptr),
          mCamera(nullptr),
          // TPS rig
          mCamPivot(nullptr),
          mCamYawNode(nullptr),
          mCamPitchNode(nullptr),
          mCamEye(nullptr),
          mCamYaw(0.0f),
          mCamPitch(-0.35f),           // <— инвертировали: отрицательный наклон = камера сверху
          mCamTargetHeight(2.0f),
          mCamDistance(6.0f),
          mCamDistCurrent(6.0f),
          mCamMinDist(2.0f),
          mCamMaxDist(20.0f),
          mCamSmooth(12.0f),
          mCamFollowYawSmooth(8.0f),
          mRmbDown(false),
          mMouseSensitivity(0.002f),
          // grid
          mGridSize(80),
          mCellSize(2.0f),
          mMapHalfSize((mGridSize * mCellSize) * 0.5f),
          // input
          mForward(false), mBackward(false), mLeft(false), mRight(false),
          // player
          mPlayerNode(nullptr),
          mPlayerEntity(nullptr),
          mPlayerYaw(0.0f),
          mCurrentSpeed(0.0f),
          mMaxForwardSpeed(40.0f),
          mMaxBackwardSpeed(20.0f),
          mAcceleration(45.0f),
          mBrakeDecel(70.0f),
          mFriction(25.0f),
          mTurnSpeed(2.8f),
          mCurrentLean(0.0f),
          mMaxLeanAngle(Degree(35.0f).valueRadians()),
          mLeanSpeed(6.0f),
          mGameTime(0.0f)
    {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }

    void setup() override
    {
        ApplicationContext::setup();
        addInputListener(this);

        mWindow = getRenderWindow();
        Root* root = getRoot();

        mSceneManager = root->createSceneManager();
        if (!RTShader::ShaderGenerator::getSingletonPtr())
            RTShader::ShaderGenerator::initialize();
        auto* sg = RTShader::ShaderGenerator::getSingletonPtr();
        sg->addSceneManager(mSceneManager);
        MaterialManager::getSingleton().setActiveScheme(
            RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

        // camera + viewport
        mCamera = mSceneManager->createCamera("MainCamera");
        mCamera->setNearClipDistance(0.1f);
        mCamera->setFarClipDistance(1000.0f);
        mCamera->setAutoAspectRatio(true);
        Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(ColourValue(0.0f, 0.0f, 0.05f));

        // TPS rig: Pivot -> Yaw -> Pitch -> Eye(Camera)
        mCamPivot     = mSceneManager->getRootSceneNode()->createChildSceneNode("CamPivot");
        mCamYawNode   = mCamPivot->createChildSceneNode("CamYaw");
        mCamPitchNode = mCamYawNode->createChildSceneNode("CamPitch");
        mCamEye       = mCamPitchNode->createChildSceneNode("CamEye");
        mCamEye->attachObject(mCamera);

        // исходные углы
        mCamYawNode->setOrientation(Quaternion(Radian(mCamYaw),   Vector3::UNIT_Y));
        mCamPitchNode->setOrientation(Quaternion(Radian(mCamPitch), Vector3::UNIT_X));
        mCamEye->setPosition(0, 0, +mCamDistCurrent); // камера на +Z смотрит на Pivot

        // lights
        {
            Light* mainLight = mSceneManager->createLight("MainLight");
            mainLight->setType(Light::LT_DIRECTIONAL);
            SceneNode* lightNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
            lightNode->attachObject(mainLight);
            lightNode->setDirection(Vector3(-0.5f, -1.0f, -0.3f).normalisedCopy());
            mainLight->setDiffuseColour(ColourValue(0.6f, 0.6f, 0.6f));

            Light* neon = mSceneManager->createLight("Neon");
            neon->setType(Light::LT_POINT);
            SceneNode* n = mSceneManager->getRootSceneNode()->createChildSceneNode();
            n->attachObject(neon);
            n->setPosition(0, 30, 0);
            neon->setDiffuseColour(ColourValue(0.1f, 0.2f, 0.6f));
            neon->setSpecularColour(ColourValue(0.1f, 0.2f, 0.6f));
        }

        createGrid();
        createPlayer();

        root->addFrameListener(this);

        std::cout << "ПКМ — вращать камеру, колёсико — зум. Камера следует за кубиком.\n";
        std::cout << "W/S — вперёд/назад, A/D — поворот, ESC — выход\n";
    }

    bool frameRenderingQueued(const FrameEvent& evt) override
    {
        if (mWindow->isClosed()) return false;

        float dt = evt.timeSinceLastFrame;
        mGameTime += dt;

        updatePlayerMovement(dt);
        updateCameraRig(dt);

        return true;
    }

    // ---------- input ----------
    bool keyPressed(const KeyboardEvent& e) override
    {
        switch (e.keysym.sym)
        {
            case SDLK_w: mForward  = true; break;
            case SDLK_s: mBackward = true; break;
            case SDLK_a: mLeft     = true; break;
            case SDLK_d: mRight    = true; break;
            default: break;
        }
        return true;
    }
    bool keyReleased(const KeyboardEvent& e) override
    {
        switch (e.keysym.sym)
        {
            case SDLK_w: mForward  = false; break;
            case SDLK_s: mBackward = false; break;
            case SDLK_a: mLeft     = false; break;
            case SDLK_d: mRight    = false; break;
            default: break;
        }
        return true;
    }
    bool mousePressed(const MouseButtonEvent& e) override
    {
        if (e.button == BUTTON_RIGHT) mRmbDown = true;
        return true;
    }
    bool mouseReleased(const MouseButtonEvent& e) override
    {
        if (e.button == BUTTON_RIGHT) mRmbDown = false;
        return true;
    }
    bool mouseMoved(const MouseMotionEvent& e) override
    {
        if (mRmbDown)
        {
            mCamYaw   -= static_cast<float>(e.xrel) * mMouseSensitivity;
            mCamPitch -= static_cast<float>(e.yrel) * mMouseSensitivity; // отрицательная — «сверху»

            const float pitchMin = Degree(-85).valueRadians();
            const float pitchMax = Degree( 85).valueRadians();
            if (mCamPitch < pitchMin) mCamPitch = pitchMin;
            if (mCamPitch > pitchMax) mCamPitch = pitchMax;

            mCamYawNode->setOrientation(Quaternion(Radian(mCamYaw),   Vector3::UNIT_Y));
            mCamPitchNode->setOrientation(Quaternion(Radian(mCamPitch), Vector3::UNIT_X));
        }
        return true;
    }
    bool mouseWheelRolled(const MouseWheelEvent& e) override
    {
        // e.y > 0 — к себе
        mCamDistance -= static_cast<float>(e.y) * 1.0f;
        if (mCamDistance < mCamMinDist) mCamDistance = mCamMinDist;
        if (mCamDistance > mCamMaxDist) mCamDistance = mCamMaxDist;
        return true;
    }

private:
    // ogre
    SceneManager*  mSceneManager;
    RenderWindow*  mWindow;
    Camera*        mCamera;

    // TPS rig
    SceneNode* mCamPivot;
    SceneNode* mCamYawNode;
    SceneNode* mCamPitchNode;
    SceneNode* mCamEye;
    float mCamYaw, mCamPitch;
    float mCamTargetHeight;
    float mCamDistance, mCamDistCurrent;
    float mCamMinDist, mCamMaxDist;
    float mCamSmooth;
    float mCamFollowYawSmooth;
    bool  mRmbDown;
    float mMouseSensitivity;

    // grid
    int   mGridSize;
    float mCellSize;
    float mMapHalfSize;

    // input
    bool mForward, mBackward, mLeft, mRight;

    // player
    SceneNode* mPlayerNode;
    Entity*    mPlayerEntity;
    float mPlayerYaw;
    float mCurrentSpeed;
    float mMaxForwardSpeed, mMaxBackwardSpeed;
    float mAcceleration, mBrakeDecel, mFriction, mTurnSpeed;
    float mCurrentLean, mMaxLeanAngle, mLeanSpeed;

    // time
    float mGameTime;

private:
    // ---------- world ----------
    void createGrid()
    {
        // ground
        ManualObject* ground = mSceneManager->createManualObject("Ground");
        ground->begin("BaseWhiteNoLighting", RenderOperation::OT_TRIANGLE_LIST);

        float half = mMapHalfSize;
        for (int x = 0; x < mGridSize; ++x)
        for (int z = 0; z < mGridSize; ++z)
        {
            float x0 = x * mCellSize - half, x1 = x0 + mCellSize;
            float z0 = z * mCellSize - half, z1 = z0 + mCellSize;

            Vector3 v0(x0, 0, z0), v1(x1, 0, z0), v2(x1, 0, z1), v3(x0, 0, z1);
            size_t base = ground->getCurrentVertexCount();
            ground->position(v0); ground->normal(Vector3::UNIT_Y);
            ground->position(v1); ground->normal(Vector3::UNIT_Y);
            ground->position(v2); ground->normal(Vector3::UNIT_Y);
            ground->position(v3); ground->normal(Vector3::UNIT_Y);
            ground->triangle(base+0, base+1, base+2);
            ground->triangle(base+0, base+2, base+3);
        }
        ground->end();
        auto* gnode = mSceneManager->getRootSceneNode()->createChildSceneNode();
        gnode->attachObject(ground);

        MaterialPtr gmat = MaterialManager::getSingleton().create(
            "GroundMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        auto* pass = gmat->getTechnique(0)->getPass(0);
        pass->setDiffuse(ColourValue(0.03f, 0.03f, 0.08f));
        pass->setAmbient(ColourValue(0.01f, 0.01f, 0.03f));
        ground->setMaterialName(0, "GroundMaterial");

        // grid lines
        ManualObject* grid = mSceneManager->createManualObject("GridLines");
        grid->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);
        for (int i = 0; i <= mGridSize; ++i)
        {
            float p = i * mCellSize - half;
            grid->position(-half, 0.02f, p); grid->position(+half, 0.02f, p);
            grid->position(p, 0.02f, -half); grid->position(p, 0.02f, +half);
        }
        grid->end();
        auto* lnode = mSceneManager->getRootSceneNode()->createChildSceneNode();
        lnode->attachObject(grid);

        MaterialPtr lmat = MaterialManager::getSingleton().create(
            "GridMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        auto* lp = lmat->getTechnique(0)->getPass(0);
        lp->setDiffuse(ColourValue(0.0f, 0.5f, 1.0f));
        lp->setAmbient(ColourValue(0.0f, 0.15f, 0.3f));
        lp->setSelfIllumination(ColourValue(0.0f, 0.4f, 1.0f));
        grid->setMaterialName(0, "GridMaterial");
    }

    Entity* createBoxEntity(const String& meshName, const String& entName,
                            float L=2.0f, float W=2.0f, float H=2.0f,
                            const ColourValue& color=ColourValue(0.8f,0.8f,1.0f))
    {
        ManualObject* m = mSceneManager->createManualObject(entName + "_MO");
        m->begin("BaseWhiteNoLighting", RenderOperation::OT_TRIANGLE_LIST);

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
        addQuad(0,1,2,3); addQuad(4,7,6,5);
        addQuad(0,3,7,4); addQuad(1,5,6,2);
        addQuad(0,4,5,1); addQuad(3,2,6,7);
        m->end();

        String matName = entName + "_Mat";
        MaterialPtr mat = MaterialManager::getSingleton().create(
            matName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        auto* pass = mat->getTechnique(0)->getPass(0);
        pass->setDiffuse(color);
        pass->setAmbient(color * 0.5f);
        pass->setSelfIllumination(color * 0.3f);
        pass->setLightingEnabled(true);

        m->setMaterialName(0, matName);

        MeshPtr mesh = m->convertToMesh(meshName);
        mSceneManager->destroyManualObject(m);

        Entity* ent = mSceneManager->createEntity(entName, meshName);
        ent->setMaterialName(matName);
        return ent;
    }

    void createPlayer()
    {
        mPlayerEntity = createBoxEntity("PlayerBoxMesh", "PlayerBoxEntity",
                                        2.5f, 1.6f, 1.6f, ColourValue(0.2f, 0.7f, 1.0f));
        mPlayerNode = mSceneManager->getRootSceneNode()->createChildSceneNode("PlayerNode");
        mPlayerNode->attachObject(mPlayerEntity);

        AxisAlignedBox bb = mPlayerEntity->getBoundingBox();
        float raise = -bb.getMinimum().y;
        mPlayerNode->translate(0, raise, 0);

        mPlayerNode->setPosition(0, 0, 0);
        mPlayerYaw = 0.0f;
        mPlayerNode->setOrientation(Quaternion(Radian(mPlayerYaw), Vector3::UNIT_Y));
    }

    // ---------- movement ----------
    void updatePlayerMovement(float dt)
    {
        float acc = 0.0f;
        if (mForward)  acc += mAcceleration;
        if (mBackward) acc += (mCurrentSpeed > 0.0f ? -mBrakeDecel : -mAcceleration);

        if (mForward || mBackward) mCurrentSpeed += acc * dt;
        else {
            if (mCurrentSpeed > 0.0f) { mCurrentSpeed -= mFriction * dt; if (mCurrentSpeed < 0) mCurrentSpeed = 0; }
            else if (mCurrentSpeed < 0.0f) { mCurrentSpeed += mFriction * dt; if (mCurrentSpeed > 0) mCurrentSpeed = 0; }
        }
        if (mCurrentSpeed >  mMaxForwardSpeed)  mCurrentSpeed =  mMaxForwardSpeed;
        if (mCurrentSpeed < -mMaxBackwardSpeed) mCurrentSpeed = -mMaxBackwardSpeed;

        float turn = 0.0f;
        if (mLeft)  turn += 1.0f;
        if (mRight) turn -= 1.0f;

        float dirSign = (mCurrentSpeed >= 0.0f ? 1.0f : -1.0f);
        mPlayerYaw += turn * mTurnSpeed * dt * dirSign;

        float speedFactor = std::min(1.0f, std::fabs(mCurrentSpeed) / mMaxForwardSpeed);
        float targetLean = turn * mMaxLeanAngle * speedFactor;
        float lerp = std::min(1.0f, mLeanSpeed * dt);
        mCurrentLean += (targetLean - mCurrentLean) * lerp;

        Quaternion yawQ(Radian(mPlayerYaw), Vector3::UNIT_Y);
        Quaternion rollQ(Radian(mCurrentLean), Vector3::UNIT_Z);
        mPlayerNode->setOrientation(yawQ * rollQ);

        Vector3 forward = yawQ * Vector3(0, 0, -1);
        Vector3 delta   = forward * (mCurrentSpeed * dt);
        Vector3 newPos  = mPlayerNode->getPosition() + delta;

        float border = mMapHalfSize - 2.0f;
        newPos.x = std::max(-border, std::min(border, newPos.x));
        newPos.z = std::max(-border, std::min(border, newPos.z));
        mPlayerNode->setPosition(newPos);
    }

    // ---------- camera follow ----------
    static float wrapPi(float a)
    {
        while (a >  Math::PI) a -= Math::TWO_PI;
        while (a < -Math::PI) a += Math::TWO_PI;
        return a;
    }

    void updateCameraRig(float dt)
    {
        if (!mPlayerNode) return;

        // pivot на уровне головы куба
        Vector3 target = mPlayerNode->getPosition() + Vector3(0, mCamTargetHeight, 0);
        mCamPivot->setPosition(target);

        // если ПКМ не зажата — камера мягко догоняет поворот куба (вид «сзади»)
        if (!mRmbDown)
        {
            float diff = wrapPi(mPlayerYaw - mCamYaw);
            float t = 1.0f - std::exp(-mCamFollowYawSmooth * dt);
            mCamYaw += diff * t;
            mCamYawNode->setOrientation(Quaternion(Radian(mCamYaw), Vector3::UNIT_Y));
        }

        // плавный зум
        float tz = 1.0f - std::exp(-mCamSmooth * dt);
        mCamDistCurrent += (mCamDistance - mCamDistCurrent) * tz;

        // применяем pitch (уже пересчитывается в mouseMoved при ПКМ)
        mCamPitchNode->setOrientation(Quaternion(Radian(mCamPitch), Vector3::UNIT_X));

        // камера стоит на +Z относительно pitch-узла и «смотрит» на Pivot
        mCamEye->setPosition(0, 0, +mCamDistCurrent);
    }
};

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
