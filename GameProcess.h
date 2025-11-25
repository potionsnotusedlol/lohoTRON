#ifndef GAMEPROCESS_H
#define GAMEPROCESS_H

#include <memory>

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

    void updateSimulation(float dt);
    void updateCamera(float dt);

    void drawScene3D();
    void drawGroundGrid();
    void drawBike();

    void setupProjection();
    void setupView();

    static float clampf(float v, float lo, float hi);
    static float lerpf(float a, float b, float t);
    static float wrapPi(float a);

private:
    std::unique_ptr<Ogre::Root> m_root;
    Ogre::SceneManager*  m_scene_manager  = nullptr;
    Ogre::RenderWindow*  m_render_window  = nullptr;

    int   m_fieldSize   = 10;   
    int   m_gridSize    = 10;   
    float m_cellSize    = 2.0f; 
    float m_mapHalfSize = 10.0f;

    bool  m_paused = false;

    Bike  m_bike;

    float     m_camYaw   = 0.0f;
    float     m_camPitch = -0.35f;
    float     m_camDistance    = 8.0f;
    float     m_camDistanceCur = 8.0f;
    float     m_camTargetHeight    = 2.0f;
    float     m_camSmooth          = 12.0f;
    float     m_camFollowYawSmooth = 8.0f;
    QVector3D m_camTarget;

    bool   m_rmbDown = false;
    QPoint m_lastMousePos;
    float  m_mouseSensitivity = 0.003f;

    bool m_keyForward  = false;
    bool m_keyBackward = false;
    bool m_keyLeft     = false;
    bool m_keyRight    = false;

    float m_maxForwardSpeed  = 40.0f;
    float m_maxBackwardSpeed = 20.0f;
    float m_acceleration     = 45.0f;
    float m_brakeDecel       = 70.0f;
    float m_friction         = 25.0f;
    float m_turnSpeed        = 2.8f;
    float m_maxLeanAngle     = Ogre::Degree(35.0f).valueRadians();
    float m_leanSpeed        = 6.0f;

    QElapsedTimer m_timer;
    qint64        m_lastTimeMs = 0;
    QTimer*       m_tickTimer  = nullptr;
};

#endif // GAMEPROCESS_H
