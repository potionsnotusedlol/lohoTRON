#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "MainMenuWidget.h"
#include "GameProcess.h"

class mainwindow : public QMainWindow {
    Q_OBJECT
public:
    explicit mainwindow(QWidget * parent = nullptr);
public slots:
    void showMenu();
    void startGame(int fieldSize, int botsCount, int roundsCount);
private:
    QStackedWidget* stacked;
    MainMenuWidget* menu;
    GameProcess* game_proc_window;
    // GameOverWindow* gameOverWindow = nullptr;
};

#endif // MAINWINDOW_H