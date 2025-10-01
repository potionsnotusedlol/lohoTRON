#include <memory>
#include <string>
#include <map>
#include <functional>
#include <vector>

struct Color {
    int r = 0, g = 0, b = 0;
    Color(int red = 0, int green = 0, int blue = 0) : r(red), g(green), b(blue) {}
    std::string toString() const {
        return "RGB(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
    }
};

struct Size {
    int width = 1024, height = 768;
    Size(int w = 1024, int h = 768) : width(w), height(h) {}
    std::string toString() const { return std::to_string(width) + "x" + std::to_string(height); }
};

struct KeyBinding {
    std::string keyName;
    int keyCode = 0;
    KeyBinding(const std::string& name = "", int code = 0) : keyName(name), keyCode(code) {}
    std::string toString() const { return keyName + "(" + std::to_string(keyCode) + ")"; }
};

struct GameConfig {
    int rounds = 1;
    int botsCount = 3;
    std::string level = "medium";
    
    bool isTrainingMode() const { return botsCount == 0; }
    std::string toString() const {
        return "Раунды: " + std::to_string(rounds) + ", Боты: " + std::to_string(botsCount) + ", Уровень: " + level;
    }
};

struct PlayerConfig {
    std::string name = "Игрок";
    Color color = Color(0, 0, 255);
    int characterType = 0;
    
    std::string toString() const {
        return "Имя: " + name + ", Цвет: " + color.toString();
    }
};

struct ControlsConfig {
    KeyBinding moveUp = KeyBinding("W", 87);
    KeyBinding moveDown = KeyBinding("S", 83);
    KeyBinding moveLeft = KeyBinding("A", 65);
    KeyBinding moveRight = KeyBinding("D", 68);
    KeyBinding action = KeyBinding("Space", 32);
    
    std::string toString() const {
        return "Управление: Вверх=" + moveUp.toString() + ", Вниз=" + moveDown.toString() + 
               ", Влево=" + moveLeft.toString() + ", Вправо=" + moveRight.toString() + 
               ", Действие=" + action.toString();
    }
};

struct VideoConfig {
    Size resolution = Size(1024, 768);
    bool fullscreen = false;
    int quality = 2;
    
    std::string toString() const {
        return "Разрешение: " + resolution.toString() + ", Полный экран: " + 
               (fullscreen ? "Да" : "Нет") + ", Качество: " + std::to_string(quality);
    }
};

enum class MenuSection {
    MAIN_MENU,
    GAME_SETTINGS,
    PLAYER_SETTINGS,
    CONTROLS_SETTINGS,
    VIDEO_SETTINGS,
    ABOUT,
    IN_GAME,
    PAUSE_MENU,
    EXIT_CONFIRMATION
};

enum class MenuAction {
    START_GAME,
    OPEN_GAME_SETTINGS,
    OPEN_PLAYER_SETTINGS,
    OPEN_CONTROLS_SETTINGS,
    OPEN_VIDEO_SETTINGS,
    OPEN_ABOUT,
    BACK_TO_MAIN,
    SAVE_SETTINGS,
    APPLY_SETTINGS,
    EXIT_GAME,
    CONFIRM_EXIT,
    CANCEL_EXIT,
    RESUME_GAME,
    RESTART_GAME,
    PAUSE_GAME
};

struct MenuCallbacks {
    std::function<void(MenuSection)> onSectionChanged; 
    std::function<void(const std::string&)> onButtonPressed; 
    std::function<void(const std::string&)> onNavigation; 
    std::function<void()> onGameStart;
    std::function<void()> onGamePause;
    std::function<void()> onGameResume;
    std::function<void()> onGameExit;
    std::function<void(const std::string&)> onSettingsSaved;
};

class MenuBackend {
private:
    MenuSection currentSection = MenuSection::MAIN_MENU;
    MenuSection previousSection = MenuSection::MAIN_MENU;
    
    GameConfig currentGameConfig;
    PlayerConfig currentPlayerConfig;
    ControlsConfig currentControlsConfig;
    VideoConfig currentVideoConfig;
    
    MenuCallbacks callbacks;
    bool gameRunning = false;
    bool gamePaused = false;
    
    std::vector<MenuSection> navigationHistory;

public:
    void setCallbacks(const MenuCallbacks& cb) {
        callbacks = cb;
    }
    
    // Обработка нажатия кнопки/действия
    void handleAction(MenuAction action) {
        std::string actionName = getActionName(action);
        
        if (callbacks.onButtonPressed) {
            callbacks.onButtonPressed(actionName);
        }
        
        switch (action) {
            case MenuAction::START_GAME:
                startGame();
                break;
            case MenuAction::OPEN_GAME_SETTINGS:
                openSection(MenuSection::GAME_SETTINGS);
                break;
            case MenuAction::OPEN_PLAYER_SETTINGS:
                openSection(MenuSection::PLAYER_SETTINGS);
                break;
            case MenuAction::OPEN_CONTROLS_SETTINGS:
                openSection(MenuSection::CONTROLS_SETTINGS);
                break;
            case MenuAction::OPEN_VIDEO_SETTINGS:
                openSection(MenuSection::VIDEO_SETTINGS);
                break;
            case MenuAction::OPEN_ABOUT:
                openSection(MenuSection::ABOUT);
                break;
            case MenuAction::BACK_TO_MAIN:
                openSection(MenuSection::MAIN_MENU);
                break;
            case MenuAction::RESUME_GAME:
                resumeGame();
                break;
            case MenuAction::PAUSE_GAME:
                pauseGame();
                break;
                
            case MenuAction::SAVE_SETTINGS:
                saveSettings();
                break;
            case MenuAction::APPLY_SETTINGS:
                applySettings();
                break;
                
            case MenuAction::EXIT_GAME:
                openSection(MenuSection::EXIT_CONFIRMATION);
                break;
            case MenuAction::CONFIRM_EXIT:
                exitGame();
                break;
            case MenuAction::CANCEL_EXIT:
                goBack();
                break;
                
            case MenuAction::RESTART_GAME:
                restartGame();
                break;
        }
    }
    
    void openSection(MenuSection section) {
        if (section == currentSection) return;
        
        previousSection = currentSection;
        currentSection = section;
        navigationHistory.push_back(previousSection);
        
        if (callbacks.onSectionChanged) {
            callbacks.onSectionChanged(section);
        }
        
        std::string sectionName = getSectionName(section);
        if (callbacks.onNavigation) {
            callbacks.onNavigation("Открыта секция: " + sectionName);
        }
    }
    
    void goBack() {
        if (!navigationHistory.empty()) {
            MenuSection targetSection = navigationHistory.back();
            navigationHistory.pop_back();
            openSection(targetSection);
        } else {
            openSection(MenuSection::MAIN_MENU);
        }
    }
    
    void startGame() {
        if (gameRunning) return;
        
        gameRunning = true;
        gamePaused = false;
        openSection(MenuSection::IN_GAME);
        
        if (callbacks.onGameStart) {
            callbacks.onGameStart();
        }
        
        if (callbacks.onNavigation) {
            callbacks.onNavigation("Игра началась");
        }
    }
    
    void pauseGame() {
        if (!gameRunning || gamePaused) return;
        
        gamePaused = true;
        openSection(MenuSection::PAUSE_MENU);
        
        if (callbacks.onGamePause) {
            callbacks.onGamePause();
        }
    }
    
    void resumeGame() {
        if (!gameRunning || !gamePaused) return;
        
        gamePaused = false;
        openSection(MenuSection::IN_GAME);
        
        if (callbacks.onGameResume) {
            callbacks.onGameResume();
        }
    }
    
    void restartGame() {
        if (callbacks.onNavigation) {
            callbacks.onNavigation("Перезапуск игры");
        }
        startGame();
    }
    
    void exitGame() {
        gameRunning = false;
        gamePaused = false;
        
        if (callbacks.onGameExit) {
            callbacks.onGameExit();
        }
        
        if (callbacks.onNavigation) {
            callbacks.onNavigation("Выход из игры");
        }
    }
    
    void setGameRounds(int rounds) {
        if (rounds >= 1 && rounds <= 10) {
            currentGameConfig.rounds = rounds;
            notifySettingsChanged("Количество раундов изменено: " + std::to_string(rounds));
        }
    }
    
    void setBotsCount(int count) {
        if (count >= 0 && count <= 10) {
            currentGameConfig.botsCount = count;
            notifySettingsChanged("Количество ботов изменено: " + std::to_string(count));
        }
    }
    
    void setGameLevel(const std::string& level) {
        if (level == "easy" || level == "medium" || level == "hard") {
            currentGameConfig.level = level;
            notifySettingsChanged("Уровень сложности изменен: " + level);
        }
    }
    
    void setPlayerName(const std::string& name) {
        if (!name.empty()) {
            currentPlayerConfig.name = name;
            notifySettingsChanged("Имя игрока изменено: " + name);
        }
    }
    
    void setPlayerColor(const Color& color) {
        if (!(color.r == 255 && color.g == 255 && color.b == 0)) { 
            currentPlayerConfig.color = color;
            notifySettingsChanged("Цвет игрока изменен");
        }
    }
    
    void saveSettings() {
        if (callbacks.onSettingsSaved) {
            callbacks.onSettingsSaved("Настройки сохранены");
        }
        
        if (callbacks.onNavigation) {
            callbacks.onNavigation("Настройки сохранены, возврат в главное меню");
        }
        
        openSection(MenuSection::MAIN_MENU);
    }
    
    void applySettings() {
        if (callbacks.onNavigation) {
            callbacks.onNavigation("Настройки применены");
        }
    }

    MenuSection getCurrentSection() const { return currentSection; }
    MenuSection getPreviousSection() const { return previousSection; }
    bool isGameRunning() const { return gameRunning; }
    bool isGamePaused() const { return gamePaused; }
    
    GameConfig getGameConfig() const { return currentGameConfig; }
    PlayerConfig getPlayerConfig() const { return currentPlayerConfig; }
    ControlsConfig getControlsConfig() const { return currentControlsConfig; }
    VideoConfig getVideoConfig() const { return currentVideoConfig; }
    
    std::vector<MenuAction> getAvailableActions() const {
        switch (currentSection) {
            case MenuSection::MAIN_MENU:
                return {MenuAction::START_GAME, MenuAction::OPEN_GAME_SETTINGS, 
                       MenuAction::OPEN_PLAYER_SETTINGS, MenuAction::OPEN_CONTROLS_SETTINGS,
                       MenuAction::OPEN_VIDEO_SETTINGS, MenuAction::OPEN_ABOUT, MenuAction::EXIT_GAME};
            
            case MenuSection::GAME_SETTINGS:
            case MenuSection::PLAYER_SETTINGS:
            case MenuSection::CONTROLS_SETTINGS:
            case MenuSection::VIDEO_SETTINGS:
                return {MenuAction::SAVE_SETTINGS, MenuAction::APPLY_SETTINGS, MenuAction::BACK_TO_MAIN};
            
            case MenuSection::ABOUT:
                return {MenuAction::BACK_TO_MAIN};
            
            case MenuSection::IN_GAME:
                return {MenuAction::PAUSE_GAME, MenuAction::EXIT_GAME};
            
            case MenuSection::PAUSE_MENU:
                return {MenuAction::RESUME_GAME, MenuAction::RESTART_GAME, MenuAction::EXIT_GAME};
            
            case MenuSection::EXIT_CONFIRMATION:
                return {MenuAction::CONFIRM_EXIT, MenuAction::CANCEL_EXIT};
        }
        return {};
    }
    
    std::string getCurrentSectionInfo() const {
        return "Текущая секция: " + getSectionName(currentSection) + 
               (gameRunning ? " (Игра активна)" : "") + 
               (gamePaused ? " (Игра на паузе)" : "");
    }

private:
    void notifySettingsChanged(const std::string& message) {
        if (callbacks.onNavigation) {
            callbacks.onNavigation(message);
        }
    }
    
    std::string getSectionName(MenuSection section) const {
        switch (section) {
            case MenuSection::MAIN_MENU: return "Главное меню";
            case MenuSection::GAME_SETTINGS: return "Настройки игры";
            case MenuSection::PLAYER_SETTINGS: return "Настройки игрока";
            case MenuSection::CONTROLS_SETTINGS: return "Настройки управления";
            case MenuSection::VIDEO_SETTINGS: return "Настройки графики";
            case MenuSection::ABOUT: return "О программе";
            case MenuSection::IN_GAME: return "Игровой процесс";
            case MenuSection::PAUSE_MENU: return "Меню паузы";
            case MenuSection::EXIT_CONFIRMATION: return "Подтверждение выхода";
        }
        return "Неизвестная секция";
    }
    
    std::string getActionName(MenuAction action) const {
        switch (action) {
            case MenuAction::START_GAME: return "Начать игру";
            case MenuAction::OPEN_GAME_SETTINGS: return "Настройки игры";
            case MenuAction::OPEN_PLAYER_SETTINGS: return "Настройки игрока";
            case MenuAction::OPEN_CONTROLS_SETTINGS: return "Настройки управления";
            case MenuAction::OPEN_VIDEO_SETTINGS: return "Настройки графики";
            case MenuAction::OPEN_ABOUT: return "О программе";
            case MenuAction::BACK_TO_MAIN: return "В главное меню";
            case MenuAction::SAVE_SETTINGS: return "Сохранить настройки";
            case MenuAction::APPLY_SETTINGS: return "Применить настройки";
            case MenuAction::EXIT_GAME: return "Выход";
            case MenuAction::CONFIRM_EXIT: return "Подтвердить выход";
            case MenuAction::CANCEL_EXIT: return "Отмена выхода";
            case MenuAction::RESUME_GAME: return "Продолжить игру";
            case MenuAction::RESTART_GAME: return "Перезапустить игру";
            case MenuAction::PAUSE_GAME: return "Пауза";
        }
        return "Неизвестное действие";
    }
};

class MenuController {
private:
    MenuBackend backend;
    
public:
    void initialize() {
        MenuCallbacks callbacks;
        callbacks.onSectionChanged = [](MenuSection section) {
            std::cout << "[NAVIGATION] Открыта секция: " << static_cast<int>(section) << std::endl;
        };
        callbacks.onButtonPressed = [](const std::string& button) {
            std::cout << "[BUTTON] Нажата кнопка: " << button << std::endl;
        };
        callbacks.onNavigation = [](const std::string& event) {
            std::cout << "[EVENT] " << event << std::endl;
        };
        
        backend.setCallbacks(callbacks);
    }
  
    void simulateUserSession() {
        std::cout << "меню" << std::endl;
        
        backend.handleAction(MenuAction::OPEN_GAME_SETTINGS);
        backend.setGameRounds(5);
        backend.setBotsCount(2);
        backend.handleAction(MenuAction::SAVE_SETTINGS);
        backend.handleAction(MenuAction::START_GAME);
        
        backend.handleAction(MenuAction::PAUSE_GAME);
        backend.handleAction(MenuAction::RESUME_GAME);
        
        backend.handleAction(MenuAction::PAUSE_GAME);
        backend.handleAction(MenuAction::EXIT_GAME);
        backend.handleAction(MenuAction::CONFIRM_EXIT);
    }
};