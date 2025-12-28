#ifndef GAMEPROCESS_H
#define GAMEPROCESS_H

#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <Ogre.h>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPen>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QElapsedTimer>
#include <QTimer>
#include <QVector3D>
#include <QMessageBox>
#include <QMessageBox>
#include <QPoint>
#include <QMatrix4x4>
#include "GamePauseWindow.h"
#include "SettingsWindow.h"
#include "GameOverWindow.h"




class GameProcess : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit GameProcess(QWidget* parent = nullptr);
    void setFieldSize(int n);
    void setBotCount(int n);        
    void setRoundsCount(int n);  
    unsigned short getColor() const;
public slots:
    void resetGameSlot();          
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    GamePauseWindow* pauseWindow;
signals:
    void exitToMainMenu();
    void matchOver(bool playerWin, int killedBots, int roundsWon);
private slots:
    void onTick();
    void exitToMenuInternal();
private:
GameOverWindow* gameOverWindow = nullptr;
    struct TrailPoint {
        QVector3D pos;
        float time;
    };
    struct Bike {
        QVector3D pos;
        QVector3D prevPos;
        QVector3D currPos;
        float yaw;
        float speed;
        float lean;
        QVector3D color;
        bool human;
        bool alive;
        float aiTurnTimer;
        float aiTurnDir;
    };
    void resetGame(bool newMatch);
    void updateSimulation(float dt);
    void updateCamera(float dt);
    void updateTrail(float dt);
    void setupProjection();
    void setupView();
    void drawScene3D();
    void drawGroundGrid();
    void killBike(int idx);
    void drawBike();
    void drawTrail();
    static float clampf(float v, float lo, float hi);
    static float lerpf(float a, float b, float t);
    static float wrapPi(float a);
    bool m_gameOverShown = false;

    std::unique_ptr<Ogre::Root> m_root;
    Ogre::SceneManager* m_scene_manager;
    Ogre::RenderWindow* m_render_window;
    int m_fieldSize;
    int m_gridSize;
    float m_cellSize;
    float m_mapHalfSize;
    bool m_paused;
    Bike m_bike;
    std::vector<Bike> m_bikes;
    std::vector<std::vector<TrailPoint>> m_bikeTrails;
    float m_camYaw;
    float m_camPitch;
    float m_camDistance;
    float m_camDistanceCur;
    float m_camTargetHeight;
    float m_camSmooth;
    QVector3D m_camTarget;
    bool m_rmbDown;
    bool m_mouseCaptured;
    QPoint m_lastMousePos;
    float m_mouseSensitivity;
    int m_botCount = 3;  
    int m_roundsCount = 3;   
    int m_currentRound = 1;   
    int m_totalBots = 0;   
    int m_aliveBots = 0;   
    int m_roundsWon = 0;
    int m_roundsLost = 0;
    int m_wonRounds = 0;
    int m_lostRounds = 0;
    int m_botsCrashedIntoPlayer = 0;
    bool m_keyForward;
    bool m_matchOver = false;
    bool m_keyBackward;
    bool m_keyLeft;
    bool m_keyRight;
    float m_maxForwardSpeed;
    float m_maxBackwardSpeed;
    float m_acceleration;
    float m_brakeDecel;
    float m_friction;
    float m_turnSpeed;
    float m_maxLeanAngle;
    bool m_roundOver = false;
    int m_playerRank = 0;
    int m_deadCount = 0;
    QString m_roundText = "РАУНД ЗАКОНЧЕН\nНажмите любую клавишу";
    float m_leanSpeed;
    float m_trailTTL;
    float m_trailMinDist;
    float m_trailColumnSize;
    float m_trailColumnHeight;
    QElapsedTimer m_timer;
    qint64 m_lastTimeMs;
    float m_time;
    QTimer* m_tickTimer;
};

#endif // GAMEPROCESS_H
