#pragma once
#include <memory>
#include <stack>
#include "GameConfig.h"

// Предварительное объявление MenuState
class MenuState;

// Менеджер состояний меню - управляет переходами между экранами
class MenuManager
{
public:
    // Перечисление всех возможных состояний меню
    enum class StateType {
        MAIN_MENU,      // Главное меню
        GAME_SETUP,     // Настройка игры
        SETTINGS,       // Настройки
        CREDITS,        // О создателях
        EXIT_CONFIRM,   // Подтверждение выхода
        IN_GAME         // В игре
    };

    MenuManager();
    ~MenuManager();

    // Инициализация менеджера
    bool initialize();

    // Управление состояниями
    void changeState(StateType newState);
    void pushState(StateType state);
    void popState();

    // Обновление текущего состояния
    void update(float dt);

    // Обработка событий GUI
    void handleEvent(const CEGUI::EventArgs& e);

    // Доступ к конфигурации игры
    GameConfig& config() noexcept { return m_config; }
    const GameConfig& config() const noexcept { return m_config; }

    // Запрос на выход из игры
    void requestExit() noexcept { m_exitRequested = true; }
    bool isExitRequested() const noexcept { return m_exitRequested; }

private:
    std::stack<std::unique_ptr<MenuState>> m_stateStack; 
    GameConfig m_config;                                 // Конфигурация игры
    bool m_initialized = false;                          // Флаг инициализации
    bool m_exitRequested = false;                        // Флаг запроса на выход

    // Создание состояния по типу
    std::unique_ptr<MenuState> createState(StateType type);
};