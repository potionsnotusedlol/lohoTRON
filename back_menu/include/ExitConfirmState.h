#pragma once
#include "MenuState.h"

// Состояние подтверждения выхода из игры
class ExitConfirmState final : public MenuState
{
public:
    explicit ExitConfirmState(MenuManager& mgr);

    void enter() override;
    void exit() override;
    void update(float dt) override;
    void handleEvent(const CEGUI::EventArgs& e) override;

private:
    std::vector<CEGUI::Event::Connection> m_handlers;

    // Обработчики событий
    bool onConfirmClicked(const CEGUI::EventArgs& e);
    bool onCancelClicked(const CEGUI::EventArgs& e);

    // Элементы UI
    CEGUI::Window* m_messageText = nullptr;
    CEGUI::Window* m_confirmButton = nullptr;
    CEGUI::Window* m_cancelButton = nullptr;

    // Таймер автоматического закрытия
    float m_autoCloseTimer = 0.0f;
    const float m_autoCloseTime = 10.0f;
    
    // Обновление таймера
    void updateTimer(float dt);
};