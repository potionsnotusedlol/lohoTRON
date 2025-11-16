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

void mainwindow::onStartGameRequested(int rounds, int bots) {
    qDebug() << "Starting game with rounds:" << rounds << "bots:" << bots;
    
    // Устанавливаем настройки игры
    game_proc_window->setGameSettings(rounds, bots);
    
    // Переключаемся на игровой процесс
    stacked->setCurrentWidget(game_proc_window);
    
    // Даем фокус игровому процессу (важно для обработки ввода)
    game_proc_window->setFocus();
    
    // Активируем игровой процесс
    game_proc_window->setFocusPolicy(Qt::StrongFocus);
    game_proc_window->setFocus();
    
    qDebug() << "Switched to game process, focus:" << game_proc_window->hasFocus();
}

void mainwindow::returnToMainMenu() {
    stacked->setCurrentWidget(menu);
    menu->setFocus();
}