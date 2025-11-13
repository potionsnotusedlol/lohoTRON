#include "mainwindow.h"

mainwindow::mainwindow(QWidget* parent) : QMainWindow(parent) {
    auto stacked = new QStackedWidget(this);
    auto menu = new MainMenuWidget;
    auto game_proc_window = new GameProcess;

    stacked->addWidget(menu);
    stacked->addWidget(game_proc_window);
    setCentralWidget(stacked);
}