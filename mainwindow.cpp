#include "mainwindow.h"
#include <QDebug>

mainwindow::mainwindow(QWidget* parent) : QMainWindow(parent) {
    // Создаем stacked widget
    stacked = new QStackedWidget(this);
    
    // Создаем главное меню
    menu = new MainMenuWidget();
    
    // Создаем игровой процесс (но пока не инициализируем OGRE)
    game_proc_window = new GameProcess(this);
    
    // Добавляем виджеты в stacked
    stacked->addWidget(menu);
    stacked->addWidget(game_proc_window);
    
    // Устанавливаем stacked как центральный виджет
    setCentralWidget(stacked);
    
    // Показываем главное меню по умолчанию
    stacked->setCurrentWidget(menu);
    
    // Подключаем сигнал начала игры
    connect(menu, &MainMenuWidget::startGameRequested, this, &mainwindow::onStartGameRequested);
    
    // Настройки окна
    setWindowTitle("lohoTRON");
    resize(1200, 800);
}

mainwindow::~mainwindow() {
    // Автоматическое удаление через родительскую структуру Qt
}

void mainwindow::onStartGameRequested(int rounds, int bots)
{
    qDebug() << "Starting game with rounds:" << rounds << "bots:" << bots;
    
    game_proc_window->setGameSettings(rounds, bots);
    stacked->setCurrentWidget(game_proc_window);
    
    game_proc_window->activateWindow();
    game_proc_window->setFocus(Qt::ActiveWindowFocusReason);
    
    QApplication::processEvents();
    
    // ОТЛОЖЕННАЯ активация (запуск рендера)
    QTimer::singleShot(300, game_proc_window, &GameProcess::activateGame);
    
    qDebug() << "Switched to game process";
}



void mainwindow::returnToMainMenu() {
    stacked->setCurrentWidget(menu);
    menu->setFocus();
}