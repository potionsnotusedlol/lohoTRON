#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "MainMenuWidget.h"
#include "SinglePlayerGameProcess.h"

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
    SinglePlayerGameProcess* game_proc_window;
};

#endif // MAINWINDOW_H