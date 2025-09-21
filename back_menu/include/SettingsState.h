#pragma once
#include "MenuState.h"
#include <vector>

// Состояние настроек игры - конфигурация управления, графики и звука
class SettingsState final : public MenuState
{
public:
    explicit SettingsState(MenuManager& mgr);

    void enter() override;
    void exit() override;
    void update(float dt) override;
    void handleEvent(const CEGUI::EventArgs& e) override;

private:
    std::vector<CEGUI::Event::Connection> m_handlers;

    // Обработчики событий
    bool onSaveClicked(const CEGUI::EventArgs& e);
    bool onBackClicked(const CEGUI::EventArgs& e);
    bool onLayoutChanged(const CEGUI::EventArgs& e);
    bool onPlayerNameChanged(const CEGUI::EventArgs& e);
    bool onPlayerColorChanged(const CEGUI::EventArgs& e);
    bool onKeyBindSelected(const CEGUI::EventArgs& e);
    bool onVolumeChanged(const CEGUI::EventArgs& e);
    bool onResolutionChanged(const CEGUI::EventArgs& e);
    bool onFullscreenToggled(const CEGUI::EventArgs& e);
    bool onVsyncToggled(const CEGUI::EventArgs& e);

    // Элементы UI
    CEGUI::Window* m_playerNameEditbox = nullptr;
    CEGUI::Window* m_playerColorEditbox = nullptr;
    CEGUI::Window* m_layoutCombobox = nullptr;
    CEGUI::Window* m_keybindsListbox = nullptr;
    CEGUI::Window* m_mapSizeSlider = nullptr;
    CEGUI::Window* m_masterVolumeSlider = nullptr;
    CEGUI::Window* m_sfxVolumeSlider = nullptr;
    CEGUI::Window* m_musicVolumeSlider = nullptr;
    CEGUI::Window* m_fullscreenCheckbox = nullptr;
    CEGUI::Window* m_vsyncCheckbox = nullptr;
    CEGUI::Window* m_resolutionCombobox = nullptr;
    CEGUI::Window* m_saveButton = nullptr;
    CEGUI::Window* m_backButton = nullptr;

    // Инициализация значений UI из конфига
    void setupUIValues();
    
    // Обновление списка привязок клавиш
    void updateKeybindsList();
    
    // Валидация настроек
    bool validateSettings();
};