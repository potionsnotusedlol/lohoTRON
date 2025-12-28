#include "mainwindow.h"

mainwindow::mainwindow(QWidget* parent) : QMainWindow(parent) {
    stacked = new QStackedWidget(this);
    menu = new MainMenuWidget;
    game_proc_window = new GameProcess;
    game_proc_window->music()->stop();
    stacked->addWidget(menu);
    stacked->addWidget(game_proc_window);
    setCentralWidget(stacked);
    stacked->setCurrentWidget(menu);
    connect(game_proc_window, &GameProcess::exitToMainMenu, this, &mainwindow::showMenu);
    connect(game_proc_window, &GameProcess::matchOver, this, [this](bool win, int killedBots, int roundsWon){
    gameOverWindow->setMatchResult(win, killedBots, roundsWon);
    gameOverWindow->exec();
    });
}

void mainwindow::showMenu() {
    if (stacked && menu) {
        game_proc_window->music()->stop();
        menu->music()->play();
        stacked->setCurrentWidget(menu);
    }
}

void mainwindow::startGame(int fieldSize, int botsCount,int roundsCount) {
    menu->music()->stop();
    game_proc_window->music()->play();
    Q_UNUSED(botsCount);

    if (!stacked || !game_proc_window) return;

    game_proc_window->resetGameSlot();
    game_proc_window->setFieldSize(fieldSize);
    game_proc_window->setBotCount(botsCount);
    game_proc_window->setRoundsCount(roundsCount);
    stacked->setCurrentWidget(game_proc_window);
    game_proc_window->setFocus();
}