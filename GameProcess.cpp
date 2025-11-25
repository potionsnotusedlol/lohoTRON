#include "GameProcess.h"

GameProcess::GameProcess(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

GameProcess::~GameProcess() = default;

void GameProcess::setFieldSize(int n)
{
    if (n < 1) n = 1;
    m_fieldSize = n;
    update();
}

void GameProcess::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.f, 0.f, 0.f, 1.f);
}

void GameProcess::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void GameProcess::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int n = m_fieldSize;
    if (n <= 0) return;

    const float w = width();
    const float h = height();
    const float cellW = w / n;
    const float cellH = h / n;

    QPen pen(Qt::cyan);
    pen.setWidth(1);
    p.setPen(pen);

    for (int i = 0; i <= n; ++i) {
        const float x = i * cellW;
        p.drawLine(QPointF(x, 0), QPointF(x, h));
    }

    for (int j = 0; j <= n; ++j) {
        const float y = j * cellH;
        p.drawLine(QPointF(0, y), QPointF(w, y));
    }

    if (m_paused) {
        QFont f = p.font();
        f.setPointSize(24);
        f.setBold(true);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "PAUSED");
    }
}

void GameProcess::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_paused = !m_paused;
        update();
        return;
    }

    QOpenGLWidget::keyPressEvent(event);
}
