#ifndef QUITGAMEWINDOW_H
#define QUITGAMEWINDOW_H

#include <QDialog>
#include <QWidget>
#include <QCloseEvent>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QRect>

class QuitGameWindow : public QDialog {
    Q_OBJECT
public:
    explicit QuitGameWindow(QWidget* parent = nullptr);
protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
private:
    QPropertyAnimation *fade_animation;
    bool closing = false;
};

#endif // QUITGAMEWINDOW_H