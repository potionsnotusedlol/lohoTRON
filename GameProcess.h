#ifndef GAMEPROCESS_H
#define GAMEPROCESS_H

#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>

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
#include <QPoint>
#include <QMatrix4x4>

class GameProcess : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit GameProcess(QWidget* parent = nullptr);
    ~GameProcess() override;

    void setFieldSize(int n);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void onTick();

private:
    struct Bike {
        QVector3D pos;   
        float     yaw;   
        float     speed; 
        float     lean;  
    };

    struct TrailPoint {
        QVector3D pos;
        float     time;
    };

    void updateSimulation(float dt);
    void updateCamera(float dt);
    void updateTrail(float dt);

    void setupProjection();
    void setupView();
    void drawScene3D();
    void drawGroundGrid();
    void drawBike();
    void drawTrail();

    static float clampf(float v, float lo, float hi);
    static float lerpf(float a, float b, float t);
    static float wrapPi(float a);

private:
    std::unique_ptr<Ogre::Root> m_root;
    Ogre::SceneManager*  m_scene_manager  = nullptr;
    Ogre::RenderWindow*  m_render_window  = nullptr;

    int   m_fieldSize   = 100;   
    int   m_gridSize    = 100;
    float m_cellSize    = 2.0f;  
    float m_mapHalfSize = 100.0f; 

    bool  m_paused = false;

    Bike  m_bike;

    float     m_camYaw         = 0.0f;   
    float     m_camPitch       = -0.6f;  
    float     m_camDistance    = 12.0f;  
    float     m_camDistanceCur = 12.0f;  
    float     m_camTargetHeight = 3.0f;  
    float     m_camSmooth       = 6.0f;  

    QVector3D m_camTarget;

    bool   m_rmbDown = false;
    QPoint m_lastMousePos;
    float  m_mouseSensitivity = 0.0015f;

    bool m_keyForward  = false;
    bool m_keyBackward = false;
    bool m_keyLeft     = false;
    bool m_keyRight    = false;

    float m_maxForwardSpeed  = 20.0f;
    float m_maxBackwardSpeed = 10.0f;
    float m_acceleration     = 20.0f;
    float m_brakeDecel       = 40.0f;
    float m_friction         = 15.0f;
    float m_turnSpeed        = 1.8f;  
    float m_maxLeanAngle     = Ogre::Degree(35.0f).valueRadians();
    float m_leanSpeed        = 6.0f;

    std::vector<TrailPoint> m_trail;
    float  m_trailTTL          = 5.0f;   
    float  m_trailMinDist      = 0.30f;  
    float  m_trailColumnSize   = 0.8f;   
    float  m_trailColumnHeight = 4.0f; 

    QElapsedTimer m_timer;
    qint64        m_lastTimeMs = 0;
    float         m_time       = 0.0f;
    QTimer*       m_tickTimer  = nullptr;
};

#endif // GAMEPROCESS_H
