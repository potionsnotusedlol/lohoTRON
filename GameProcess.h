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

class GameProcess : public QOpenGLWidget, protected QOpenGLFunctions
{
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
        float yaw;
        float speed;
        float lean;
    };

    struct TrailPoint {
        QVector3D pos;
        float t;
    };

    void updateSimulation(float dt);
    void updateCamera(float dt);
    void updateTrail(float dt);

    void setupProjection();
    void setupView();

    void drawScene3D();
    void drawGround();
    void drawGrid();
    void drawBike();
    void drawTrail();

    static float clampf(float v, float a, float b);
    static float lerpf(float a, float b, float t);
    static float wrapAngle(float a);

    int m_fieldSize;
    int m_gridSize;
    float m_cellSize;
    float m_mapHalfSize;

    Bike m_bike;

    bool m_keyForward;
    bool m_keyBackward;
    bool m_keyLeft;
    bool m_keyRight;

    float m_maxForwardSpeed;
    float m_maxBackwardSpeed;
    float m_accel;
    float m_brake;
    float m_friction;
    float m_turnSpeed;
    float m_maxLean;
    float m_leanSpeed;

    float m_camYaw;
    float m_camPitch;
    float m_camDistance;
    float m_camDistanceCur;
    float m_camSmooth;
    float m_camTargetHeight;
    QVector3D m_camTarget;

    float m_mouseSensitivity;
    bool m_mouseCaptured;
    QPoint m_lastMousePos;

    std::vector<TrailPoint> m_trail;
    float m_trailTTL;
    float m_trailMinDist;
    float m_trailSize;
    float m_trailHeight;

    QElapsedTimer m_timer;
    qint64 m_lastTimeMs;
    float m_time;

    QTimer* m_tickTimer;
    bool m_paused;
};

#endif // GAMEPROCESS_H
