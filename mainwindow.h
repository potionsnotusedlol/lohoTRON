#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MainMenuWidget.h"
#include "GameProcess.h"
#include <QStackedWidget>

class mainwindow : public QMainWindow {
    Q_OBJECT

public:
    mainwindow(QWidget * parent = nullptr);
    ~mainwindow();

private slots:
    void onStartGameRequested(int rounds, int bots);
    void returnToMainMenu();

private:
    QStackedWidget* stacked;
    MainMenuWidget* menu;
    GameProcess* game_proc_window;
};

#endif // MAINWINDOW_H