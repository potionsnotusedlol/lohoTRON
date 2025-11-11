#ifndef GAMEPROCESS_H
#define GAMEPROCESS_H

#include <Ogre.h>
#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLFunctions>

class GameProcess : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit GameProcess(QWidget* parent = nullptr);
    ~GameProcess();
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
private:
    std::unique_ptr<Ogre::Root> m_root;
    Ogre::SceneManager* m_scene_manager = nullptr;
    Ogre::RenderWindow* m_render_window = nullptr;
};

#endif // GAMEPROCESS_G