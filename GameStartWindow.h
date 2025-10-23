#ifndef GAMESTARTWINDOW_H
#define GAMESTARTWINDOW_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QCloseEvent>
#include <QAbstractAnimation>

class GameStartWindow : public QDialog {
    Q_OBJECT
public:
    explicit GameStartWindow(QWidget* parent = nullptr);
protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
private:
    QGraphicsOpacityEffect *opacity_effect;
    QPropertyAnimation *fade_in_animation;
    bool closing = false;
};

#endif // GAMESTARTWINDOW_H