#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>
#include <OgreTrays.h>
#include <iostream>

using namespace Ogre;
using namespace OgreBites;

class CubeApp : public ApplicationContext, public InputListener {
public:
    CubeApp() : ApplicationContext("CubeApp") {}

    SceneNode* cubeNode = nullptr;
    float speed = 100.0f;

    void setup() override {
        ApplicationContext::setup();
        addInputListener(this);

        Root* root = getRoot();
        SceneManager* scnMgr = root->createSceneManager();

        RTShader::ShaderGenerator::getSingleton().addSceneManager(scnMgr);

        // Camera
        Camera* cam = scnMgr->createCamera("MainCam");
        cam->setNearClipDistance(5);
        cam->setAutoAspectRatio(true);
        SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
        camNode->attachObject(cam);
        camNode->setPosition(0, 200, 200);
        camNode->lookAt(Vector3(0,0,0), Node::TS_PARENT);

        getRenderWindow()->addViewport(cam);

        // Light
        scnMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
        Light* light = scnMgr->createLight("MainLight");
        SceneNode* lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject(light);
        lightNode->setPosition(20, 80, 50);

        // Plane
        Plane plane(Vector3::UNIT_Y, 0);
        MeshManager::getSingleton().createPlane(
            "ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            plane, 1000, 1000, 20, 20, true, 1, 10, 10, Vector3::UNIT_Z);

        Entity* groundEntity = scnMgr->createEntity("ground");
        groundEntity->setMaterialName("Examples/BeachStones");
        groundEntity->setCastShadows(false);
        scnMgr->getRootSceneNode()->createChildSceneNode()->attachObject(groundEntity);

        // Cube
        Entity* cube = scnMgr->createEntity(SceneManager::PT_CUBE);
        cubeNode = scnMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 10, 0));
        cubeNode->setScale(Vector3(0.1, 0.1, 0.1)); // уменьшение
        cubeNode->attachObject(cube);
    }

    bool keyPressed(const KeyboardEvent& evt) override {
        if (!cubeNode) return true;

        Vector3 trans = Vector3::ZERO;
        switch (evt.keysym.sym) {
            case SDLK_w: trans.z -= speed; break;
            case SDLK_s: trans.z += speed; break;
            case SDLK_a: trans.x -= speed; break;
            case SDLK_d: trans.x += speed; break;
            case SDLK_ESCAPE: getRoot()->queueEndRendering(); break;
        }

        cubeNode->translate(trans * getDeltaTime(), Node::TS_LOCAL);
        return true;
    }

    float getDeltaTime() {
        static uint64_t lastTime = Root::getSingleton().getTimer()->getMicroseconds();
        uint64_t now = Root::getSingleton().getTimer()->getMicroseconds();
        float dt = (now - lastTime) / 1e6f;
        lastTime = now;
        return dt;
    }
};

int main(int argc, char** argv) {
    try {
        CubeApp app;
        app.initApp();
        app.getRoot()->startRendering();
        app.closeApp();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
