#pragma once
#include "MenuState.h"
#include <vector>

// Состояние настройки игровой сессии - выбор параметров перед началом игры
class GameSetupState final : public MenuState
{
public:
    explicit GameSetupState(MenuManager& mgr);

    void enter() override;
    void exit() override;
    void update(float dt) override;
    void handleEvent(const CEGUI::EventArgs& e) override;

private:
    std::vector<CEGUI::Event::Connection> m_handlers;

    // Обработчики событий
    bool onStartClicked(const CEGUI::EventArgs& e);
    bool onBackClicked(const CEGUI::EventArgs& e);
    bool onQuickMatchToggled(const CEGUI::EventArgs& e);
    bool onBotsCountChanged(const CEGUI::EventArgs& e);
    bool onRoundsCountChanged(const CEGUI::EventArgs& e);

    // Элементы UI
    CEGUI::Window* m_roundsLabel = nullptr;
    CEGUI::Window* m_roundsEditbox = nullptr;
    CEGUI::Window* m_quickMatchCheckbox = nullptr;
    CEGUI::Window* m_botsLabel = nullptr;
    CEGUI::Window* m_botsCombobox = nullptr;
    CEGUI::Window* m_startButton = nullptr;
    CEGUI::Window* m_backButton = nullptr;

    bool validateInput();
    
    void updateUIState();
};