#include "mainwindow.h"

mainwindow::mainwindow(QWidget* parent) : QMainWindow(parent) {
    auto stacked = new QStackedWidget(this);
    auto menu = new MainMenuWidget;
    auto game_proc_window = new GameProcess;

    stacked->addWidget(menu);
    stacked->addWidget(game_proc_window);
    setCentralWidget(stacked);
    
    // Connect start button to switch to game with settings
    connect(menu, &MainMenuWidget::startGameRequested, this, 
        [stacked, game_proc_window](int rounds, int bots) {
            
            stacked->setCurrentWidget(game_proc_window);
            game_proc_window->setFocus(); // Important for input
        }
    );
}