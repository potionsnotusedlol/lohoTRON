#ifndef GAMEPROCESS_H
#define GAMEPROCESS_H

#include <memory>
#include <Ogre.h>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPen>
#include <QKeyEvent>

class GameProcess : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit GameProcess(QWidget* parent = nullptr);
    ~GameProcess();

    void setFieldSize(int n);

signals:
    void gameStartRequested(int fieldSize, int botCount);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    std::unique_ptr<Ogre::Root> m_root;
    Ogre::SceneManager* m_scene_manager = nullptr;
    Ogre::RenderWindow* m_render_window = nullptr;

    int  m_fieldSize = 10;
    bool m_paused    = false;
};

#endif // GAMEPROCESS_G
