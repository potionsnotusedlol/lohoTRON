#ifndef GAMEPAUSEWINDOW_H
#define GAMEPAUSEWINDOW_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCloseEvent>

class GamePauseWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GamePauseWindow(QWidget* parent = nullptr);
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

signals:
    void resumeGame();
    void restartGame();
    void exitToMenu();

private slots:
    void onResumeClicked();
    void onRestartClicked();
    void onExitClicked();

private:
    void setupUI();
    void setupAnimations();
    void applyStyles();
    void setupConnections();

    QPropertyAnimation* fade_in_animation;
    bool closing = false;

    // UI элементы
    QLabel* pause_label;
    QPushButton* resume_button;
    QPushButton* restart_button;
    QPushButton* exit_button;
};

#endif // GAMEPAUSEWINDOW_H