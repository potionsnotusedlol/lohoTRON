#ifndef MULTIPLAYERGAMEPROCESS_H
#define MULTIPLAYERGAMEPROCESS_H

// Simple Tron-style 2-player module using Qt Widgets
// Controls:
// Player 1: W A S D
// Player 2: Arrow Keys
// Top-down 2D view, multiple rounds, win/lose logic

#include <QWidget>
#include <QDialog>
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>
#include <QSet>

struct Bike {
    QPoint pos;
    QPoint dir;
    QSet<QPoint> trail;
    bool alive = true;
};

class MultiPlayerGameProcess : public QDialog {
    Q_OBJECT
public:
    explicit MultiPlayerGameProcess(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
private slots:
    void updateGame();
private:
    const int cell = 5;
    QTimer *timer;
    Bike p1, p2;
    bool roundActive = false;
    int p1Score = 0;
    int p2Score = 0;

    void startRound();
    void moveBike(Bike &b);
    void checkCollision(Bike &b, const Bike &other);
    void drawBike(QPainter &p, const Bike &b, QColor color);
};

// Usage:
// Create a QApplication and set TronWidget as the main widget

#endif // MULTIPLAYERGAMEPROCESS_H