#include "mainwindow.h"

mainwindow::mainwindow(QWidget* parent) : QMainWindow(parent) {
    stacked = new QStackedWidget(this);
    menu = new MainMenuWidget;
    game_proc_window = new GameProcess;

    connect(game_proc_window, &GameProcess::exitToMainMenu,
            this, &mainwindow::showMenu);

    stacked->addWidget(menu);
    stacked->addWidget(game_proc_window);
    setCentralWidget(stacked);
    stacked->setCurrentWidget(menu);
}

void mainwindow::showMenu() {
    if (stacked && menu) stacked->setCurrentWidget(menu);
}

void mainwindow::startGame(int fieldSize, int botsCount) {
    Q_UNUSED(botsCount);

    if (!stacked || !game_proc_window) return;

    game_proc_window->setFieldSize(fieldSize);
    stacked->setCurrentWidget(game_proc_window);
    game_proc_window->setFocus();
}

void mainwindow::onExitToMenu()
{
    if (stacked && menu)
        stacked->setCurrentWidget(menu);
}