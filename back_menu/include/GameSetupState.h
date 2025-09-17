#pragma once                              
#include "GameConfig.h"                    
#include <third_party/cegui/cegui/include/CEGUI/CEGUI.h>                   
#include <memory>                          

class MenuManager;                         
                                          

/*класс используется для наследования остальными и является основным для меню*/
class MenuState
{
public:
    explicit MenuState(MenuManager& mgr)    
        : m_manager(mgr)                    
    {}

    virtual ~MenuState() = default;         

    virtual void enter() = 0;

    virtual void exit() = 0;

    /*для обновления кадров мигания курсоров*/
    virtual void update(float dt) = 0;

    /*обработка системных gui событий*/
    virtual void handleEvent(const CEGUI::EventArgs& e) = 0;

    GameConfig& config() noexcept;
    const GameConfig& config() const noexcept;

protected:
    MenuManager& m_manager;

    CEGUI::Window* m_root = nullptr;
};