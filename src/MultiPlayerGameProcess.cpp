#include "MultiPlayerGameProcess.h"

MultiPlayerGameProcess::MultiPlayerGameProcess(QWidget *parent) : QDialog(parent) {
    setFixedSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MultiPlayerGameProcess::updateGame);
    startRound();
}

void MultiPlayerGameProcess::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    drawBike(p, p1, Qt::cyan);
    drawBike(p, p2, Qt::yellow);

    if (!roundActive) {
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", 20));
        p.drawText(rect(), Qt::AlignCenter, QString("Round Over\nP1: %1  P2: %2\nPress Space").arg(p1Score).arg(p2Score));
    }
}

void MultiPlayerGameProcess::keyPressEvent(QKeyEvent *e) {
    if (!roundActive && e->key() == Qt::Key_Space) {
        startRound();

        return;
    }

    // Player 1
    if (e->key() == Qt::Key_W) p1.dir = {0, -cell};

    if (e->key() == Qt::Key_S) p1.dir = {0, cell};

    if (e->key() == Qt::Key_A) p1.dir = {-cell, 0};

    if (e->key() == Qt::Key_D) p1.dir = {cell, 0};

    // Player 2
    if (e->key() == Qt::Key_Up) p2.dir = {0, -cell};

    if (e->key() == Qt::Key_Down) p2.dir = {0, cell};

    if (e->key() == Qt::Key_Left) p2.dir = {-cell, 0};

    if (e->key() == Qt::Key_Right) p2.dir = {cell, 0};
}

void MultiPlayerGameProcess::updateGame() {
    if (!roundActive) return;

    moveBike(p1);
    moveBike(p2);
    checkCollision(p1, p2);
    checkCollision(p2, p1);

    if (!p1.alive || !p2.alive) {
        roundActive = false;
        timer->stop();

        if (p1.alive) p1Score++;

        if (p2.alive) p2Score++;
    }

    update();
}

void MultiPlayerGameProcess::startRound() {
    p1 = {{100, height()/2}, {cell, 0}};
    p2 = {{width()-100, height()/2}, {-cell, 0}};
    p1.trail.clear();
    p2.trail.clear();
    p1.alive = p2.alive = true;
    roundActive = true;
    timer->start(50);
    update();
}

void MultiPlayerGameProcess::moveBike(Bike &b) {
    if (!b.alive) return;

    b.trail.insert(b.pos);
    b.pos += b.dir;
}

void MultiPlayerGameProcess::checkCollision(Bike &b, const Bike &other) {
    if (!b.alive) return;

    // Wall collision
    if (!rect().contains(b.pos)) {
        b.alive = false;

        return;
    }

    // Trail collision
    if (b.trail.contains(b.pos) || other.trail.contains(b.pos)) b.alive = false;
}

void MultiPlayerGameProcess::drawBike(QPainter &p, const Bike &b, QColor color) {
    p.setPen(color);

    for (const QPoint &pt : b.trail) p.drawRect(pt.x(), pt.y(), cell, cell);

    if (b.alive) p.fillRect(b.pos.x(), b.pos.y(), cell, cell, color);
}