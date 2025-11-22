#ifndef GAMEPROCESS_H
#define GAMEPROCESS_H

#include <Ogre.h>
#include <OgreRTShaderSystem.h>
#include <OgreTrays.h>
#include <QApplication>

#include <QTime>
#include <QDebug>
#include <QCursor>
#include <QDateTime>
#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QPaintEngine>
#include <QFocusEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QMessageBox>
#include <QPoint>
#include <QGuiApplication>
#include <QScreen>

#include <deque>
#include <vector>
#include <random>
#include <ctime>
#include <cmath>
#include <iostream>

using namespace Ogre;
using namespace OgreBites;

// Утилиты
static inline float clampf(float v, float a, float b) { return std::max(a, std::min(b, v)); }
static inline float lerpf(float a, float b, float t)  { return a + (b - a) * t; }

struct Closest2D { float t; float u; float dist2; };
static Closest2D closestSegSeg2D(const Vector2& p0, const Vector2& p1, const Vector2& q0, const Vector2& q1)
{
    const float EPS = 1e-6f;
    Vector2 u = p1 - p0; Vector2 v = q1 - q0; Vector2 w = p0 - q0;
    float a = u.dotProduct(u), b = u.dotProduct(v), c = v.dotProduct(v);
    float d = u.dotProduct(w), e = v.dotProduct(w), D = a*c - b*b;
    float sn, sd = D, tn, td = D;
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
    } else if (tn > td) {
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
    SceneNode* node = nullptr;
    Entity*    ent  = nullptr;
    Light*     glow = nullptr;

    float yaw = 0.0f;
    float speed = 0.0f;
    float lean  = 0.0f;

    ColourValue color = ColourValue(0.2f, 0.7f, 1.0f);
    String      name;
    bool        human = false;
    bool        alive = true;

    BillboardChain* trail = nullptr;
    std::deque<TrailPoint> pts;
    Vector3 lastTrailPos = Vector3::ZERO;

    Vector3 framePrevPos = Vector3::ZERO;
    Vector3 frameNewPos  = Vector3::ZERO;

    bool hitWallThisFrame = false;

    float aiTurnTimer = 0.0f;
    float aiTurnDir   = 0.0f;
};

enum class GameState { Playing, Paused, RoundEnd, GameEnd };

class GameProcess : public QWidget, public TrayListener {
    Q_OBJECT

public:
    explicit GameProcess(QWidget* parent = nullptr);
    ~GameProcess() override;

    // TrayListener
    void buttonHit(Button* b) override;

    void setGameSettings(int roundsToWin, int numberOfBots) {
        mRoundsToWin = roundsToWin;
        mNumberOfBots = numberOfBots;
        std::cout << "Game settings: rounds=" << mRoundsToWin
                  << ", bots=" << mNumberOfBots << std::endl;

        if (mRoot) {
            startMatchFresh();
        }
    }

    void activateGame() {
        std::cout << "Activating game..." << std::endl;
        setFocus();
        grabKeyboard();
        grabMouse();
        setCursor(Qt::BlankCursor);

        QCursor::setPos(mapToGlobal(QPoint(width()/2, height()/2)));
        mLastMousePos = QPoint(width()/2, height()/2);

        std::cout << "Game activated - input captured" << std::endl;
    }

    void deactivateGame() {
        releaseKeyboard();
        releaseMouse();
        setCursor(Qt::ArrowCursor);
        std::cout << "Game deactivated" << std::endl;
    }

signals:
    void returnToMenuRequested();

protected:
    QPaintEngine* paintEngine() const override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    // Ogre
    Ogre::Root*         mRoot         = nullptr;
    Ogre::SceneManager* mSceneManager = nullptr;
    Ogre::RenderWindow* mRenderWindow = nullptr;
    Ogre::Camera*       mCamera       = nullptr;
    Ogre::MaterialManager::Listener* mMaterialListener;

    // TPS rig
    SceneNode* mCamPivot     = nullptr;
    SceneNode* mCamYawNode   = nullptr;
    SceneNode* mCamPitchNode = nullptr;
    SceneNode* mCamEye       = nullptr;
    float mCamYaw   = 0.0f;
    float mCamPitch = -0.35f;
    float mCamTargetHeight = 2.0f;
    float mCamDistance     = 6.0f;
    float mCamDistCurrent  = 6.0f;
    float mCamMinDist      = 2.0f;
    float mCamMaxDist      = 40.0f;
    float mCamSmooth       = 12.0f;
    float mCamFollowYawSmooth = 8.0f;
    bool  mRmbDown         = false;
    float mMouseSensitivity = 0.002f;
    QPoint mLastMousePos;

    // Grid
    int   mGridSize    = 80;
    float mCellSize    = 2.0f;
    float mMapHalfSize = (80 * 2.0f) * 0.5f;

    // Input
    bool mForward  = false;
    bool mBackward = false;
    bool mLeft     = false;
    bool mRight    = false;
    bool mSceneCreated;

    // Movement
    float mMaxForwardSpeed  = 40.0f;
    float mMaxBackwardSpeed = 20.0f;
    float mAcceleration     = 45.0f;
    float mBrakeDecel       = 70.0f;
    float mFriction         = 25.0f;
    float mTurnSpeed        = 2.8f;
    float mMaxLeanAngle     = Ogre::Degree(35.0f).valueRadians();
    float mLeanSpeed        = 6.0f;

    // Trail
    float mTrailTTL        = 5.0f;
    float mTrailMinSegDist = 0.5f;
    float mTrailWidth      = 0.7f;

    // Collisions
    float  mPlayerRadius     = 0.6f;
    size_t mSelfSkipSegments = 2;
    float  mSelfTouchEps     = 1e-3f;

    // Players
    std::vector<Player>  mPlayers;
    size_t               mHumanIndex = 0;
    std::vector<Vector3> mSpawnsStatic;
    std::mt19937         mRng{ std::random_device{}() };

    // Game state
    GameState mState       = GameState::Playing;
    int  mRoundsToWin      = 3;
    int  mNumberOfBots     = 3;
    int  mPlayerWins       = 0;
    int  mBotsWins         = 0;
    int  mRoundIndex       = 1;
    float mRoundStartTime  = 0.0f;

    // Time
    float   mGameTime = 0.0f;
    QTimer* mTimer    = nullptr;
    qint64  mLastTime = 0;

    // UI
    OgreBites::TrayManager* mTrayMgr = nullptr;
    bool mOgreInitialised = false;

    // Methods
    void setupLighting();
    void setupOgre();
    void createScene();      
    void updateGame(float dt);
    void createGrid();
    Entity* createBoxEntity(const String& meshName, const String& entName,
                            float L = 2.5f, float W = 1.6f, float H = 1.6f,
                            const ColourValue& color = ColourValue(0.8f,0.8f,1.0f));
    void spawnPlayer(bool human, const Vector3& start, const ColourValue& color, const String& name);
    float frand(float a, float b);
    void startMatchFresh();
    void startNewRoundRandom();
    std::vector<Vector3> generateRandomSpawns(int count);
    void updateMove(Player& P, float dt, bool W, bool S, bool A, bool D);
    void updateAI(Player& P, float dt);
    void updateTrail(Player& P);
    void resolveCollisionsAndDeaths();
    struct Hit { size_t idx; float alpha; bool isPlayer; };
    void killPlayer(size_t idx);
    int  botsAliveCount() const;
    static float wrapPi(float a);
    void updateCameraRig(float dt);
    void showPauseDialog();
    void hidePauseDialog();
    void resumeFromPause();
    void showGameEndDialog(bool playerWon, float roundTime);
    void hideGameEndDialog();
private:
    void setupCameraRig() {
        mCamPivot = mSceneManager->getRootSceneNode()->createChildSceneNode("CamPivot");
        mCamYawNode = mCamPivot->createChildSceneNode("CamYaw");
        mCamPitchNode = mCamYawNode->createChildSceneNode("CamPitch");
        mCamEye = mCamPitchNode->createChildSceneNode("CamEye");
        
        mCamEye->attachObject(mCamera);
        mCamYawNode->setOrientation(Ogre::Quaternion(Ogre::Radian(mCamYaw), Ogre::Vector3::UNIT_Y));
        mCamPitchNode->setOrientation(Ogre::Quaternion(Ogre::Radian(mCamPitch), Ogre::Vector3::UNIT_X));
        mCamEye->setPosition(0, 0, mCamDistCurrent);
    }

};

#endif // GAMEPROCESS_H


