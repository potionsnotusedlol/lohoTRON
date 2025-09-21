#pragma once
#include "GameConfig.h"
#include <CEGUI/CEGUI.h>
#include <memory>

class MenuManager;

class MenuState
{
public:
    explicit MenuState(MenuManager& mgr) : m_manager(mgr) {}
    virtual ~MenuState() = default;

    virtual void enter() = 0;          // Вызывается при входе в состояние
    virtual void exit() = 0;           // Вызывается при выходе из состояния
    virtual void update(float dt) = 0; // Обновление состояния каждый кадр
    virtual void handleEvent(const CEGUI::EventArgs& e) = 0; // Обработка событий CEGUI

    GameConfig& config() noexcept;
    const GameConfig& config() const noexcept;

protected:
    MenuManager& m_manager;     // Ссылка на менеджер меню
    CEGUI::Window* m_root = nullptr; // Корневое окно состояния
};