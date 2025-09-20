#pragma once
#include "MenuState.h"
#include <vector>

// Состояние с информацией о создателях игры
class CreditsState final : public MenuState
{
public:
    explicit CreditsState(MenuManager& mgr);

    void enter() override;
    void exit() override;
    void update(float dt) override;
    void handleEvent(const CEGUI::EventArgs& e) override;

private:
    std::vector<CEGUI::Event::Connection> m_handlers;

    // Обработчик события
    bool onBackClicked(const CEGUI::EventArgs& e);

    // Элементы UI
    CEGUI::Window* m_creditsText = nullptr;
    CEGUI::Window* m_backButton = nullptr;
    CEGUI::Window* m_scrollablePane = nullptr;

    // Параметры скроллинга
    float m_scrollPosition = 0.0f;
    float m_scrollSpeed = 30.0f;
    
    // Загрузка текста кредитов
    void loadCreditsText();
    
    // Обновление скроллинга
    void updateScrolling(float dt);
};