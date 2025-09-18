#pragma once
#include "MenuState.h"  
#include <vector>
#include <functional> 

class MainMenuState final : public MenuState
{
public:
    explicit MainMenuState(MenuManager& mgr);

    void enter() override;   
    void exit() override;     
    void update(float dt) override;  // можно анимировать фон, пульсацию кнопок и т.д.
    void handleEvent(const CEGUI::EventArgs& e) override;  // пока заглушку

private:
    std::vector<CEGUI::Event::Connection> m_handlers;

    /*нажатия кнопок*/
    bool onStartClicked(const CEGUI::EventArgs&);
    bool onSettingsClicked(const CEGUI::EventArgs&);
    bool onCreditsClicked(const CEGUI::EventArgs&);
    bool onExitClicked(const CEGUI::EventArgs&);

    /*для подвески колбэка*/
    void subscribeButton(const CEGUI::String& name,
                         bool (MainMenuState::*handler)(const CEGUI::EventArgs&));
};