#ifndef GAMEPROCESS_H
#define GAMEPROCESS_H

#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>
#include <OgreTrays.h>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
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
    else { sn = (b*e - c*d); tn = (a*e - b*d);
        if (sn < 0.0f) { sn = 0.0f; tn = e; td = c; }
        else if (sn > sd) { sn = sd; tn = e + b; td = c; } }
    if (tn < 0.0f) { tn = 0.0f; if (-d < 0.0f) sn = 0.0f; else if (-d > a) sn = sd; else { sn = -d; sd = a; } }
    else if (tn > td) { tn = td; if ((-d + b) < 0.0f) sn = 0.0f; else if ((-d + b) > a) sn = sd; else { sn = (-d + b); sd = a; } }
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

class GameProcess : public QOpenGLWidget, protected QOpenGLFunctions, public TrayListener {
    Q_OBJECT

public:
    explicit GameProcess(QWidget* parent = nullptr);
    ~GameProcess();

    // TrayListener methods
    void buttonHit(Button* b) override;
    void setGameSettings(int roundsToWin, int numberOfBots) {
        mRoundsToWin = roundsToWin;
        mNumberOfBots = numberOfBots;
    }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    int mNumberOfBots = 3;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    // Ogre objects
    Ogre::Root* mRoot = nullptr;
    Ogre::SceneManager* mSceneManager = nullptr;
    Ogre::RenderWindow* mRenderWindow = nullptr;
    Ogre::Camera* mCamera = nullptr;

    // TPS rig
    SceneNode* mCamPivot = nullptr;
    SceneNode* mCamYawNode = nullptr;
    SceneNode* mCamPitchNode = nullptr;
    SceneNode* mCamEye = nullptr;
    float mCamYaw = 0.0f, mCamPitch = -0.35f;
    float mCamTargetHeight = 2.0f;
    float mCamDistance = 6.0f, mCamDistCurrent = 6.0f;
    float mCamMinDist = 2.0f, mCamMaxDist = 40.0f;
    float mCamSmooth = 12.0f, mCamFollowYawSmooth = 8.0f;
    bool  mRmbDown = false;
    float mMouseSensitivity = 0.002f;
    QPoint mLastMousePos;

    // Grid
    int   mGridSize = 80;
    float mCellSize = 2.0f;
    float mMapHalfSize = (80*2.0f)*0.5f;

    // Input
    bool mForward = false, mBackward = false, mLeft = false, mRight = false;

    // Movement
    float mMaxForwardSpeed = 40.0f, mMaxBackwardSpeed = 20.0f;
    float mAcceleration = 45.0f, mBrakeDecel = 70.0f, mFriction = 25.0f, mTurnSpeed = 2.8f;
    float mMaxLeanAngle = Ogre::Degree(35.0f).valueRadians(), mLeanSpeed = 6.0f;

    // Trail
    float mTrailTTL = 5.0f, mTrailMinSegDist = 0.5f, mTrailWidth = 0.7f;

    // Collisions
    float  mPlayerRadius = 0.6f;
    size_t mSelfSkipSegments = 2;
    float  mSelfTouchEps = 1e-3f;

    // Players
    std::vector<Player> mPlayers;
    size_t mHumanIndex = 0;
    std::vector<Vector3> mSpawnsStatic;
    std::mt19937 mRng{ std::random_device{}() };

    // Game state
    GameState mState = GameState::Playing;
    int  mRoundsToWin = 3;
    int  mPlayerWins = 0, mBotsWins = 0;
    int  mRoundIndex = 1;
    float mRoundStartTime = 0.0f;

    // Time
    float mGameTime = 0.0f;
    QTimer* mTimer = nullptr;
    qint64 mLastTime = 0;

    // UI
    OgreBites::TrayManager* mTrayMgr = nullptr;

    // Methods
    void setupOgre();
    void createGrid();
    Entity* createBoxEntity(const String& meshName, const String& entName,
                           float L=2.5f, float W=1.6f, float H=1.6f,
                           const ColourValue& color=ColourValue(0.8f,0.8f,1.0f));
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
    int botsAliveCount() const;
    static float wrapPi(float a);
    void updateCameraRig(float dt);
    
    void showPauseDialog();
    void hidePauseDialog();
    void resumeFromPause();
    void showGameEndDialog(bool playerWon, float roundTime);
    void hideGameEndDialog();
    
    void updateGame(float dt);
};

#endif // GAMEPROCESS_H