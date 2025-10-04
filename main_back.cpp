#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <optional>
#include <cassert>
#include <functional>

enum GameModeType {
    REGULAR,
    FAST
};

enum ColorPlayer {
    COLOR_RED = 1,
    COLOR_GREEN,
    COLOR_BLUE, 
    COLOR_PURPLE,
    COLOR_ORANGE,
    COLOR_CYAN,
    COLOR_YELLOW
};

enum ScreenState {
    SCREEN_MAIN,
    SCREEN_START,
    SCREEN_SETTINGS,
    SCREEN_ABOUT,
    SCREEN_EXIT,
    SCREEN_PLAY
};

enum ErrorType {
    NO_ERROR,
    BAD_VALUE,
    FILE_PROBLEM,
    DATA_CONFLICT,
    COLOR_USED,
    CHECK_FAILED
};

struct OperationResult {
    ErrorType error_code;
    std::string error_text;
    
    OperationResult(ErrorType code = NO_ERROR, const std::string& text = "") : error_code(code), error_text(text) {}
    bool good() const { return error_code == NO_ERROR; }
};

struct EventData {
    bool status;
    std::string info;
};

class EventSystem {
private:
    std::map<std::string, std::vector<std::function<void(const EventData&)>>> event_handlers;
    std::mutex lock_guard;
    
public:
    void addListener(const std::string& event_name, std::function<void(const EventData&)> handler_func) {
        std::lock_guard<std::mutex> lock(lock_guard);
        event_handlers[event_name].push_back(handler_func);
    }
    
    void sendEvent(const std::string& event_name, const EventData& event_info) {
        std::lock_guard<std::mutex> lock(lock_guard);
        auto found = event_handlers.find(event_name);
        if (found != event_handlers.end()) {
            for (auto& handler : found->second) {
                handler(event_info);
            }
        }
    }
};

class KeyboardManager {
private:
    std::map<std::string, std::map<char, std::string>> key_presets = {
        {"WASD_KEYS", {{'W', "MoveUp"}, {'A', "MoveLeft"}, {'S', "MoveDown"}, {'D', "MoveRight"}}},
        {"ARROW_KEYS", {{'U', "MoveUp"}, {'H', "MoveLeft"}, {'J', "MoveDown"}, {'K', "MoveRight"}}},
        {"NUM_PAD", {{'8', "MoveUp"}, {'4', "MoveLeft"}, {'2', "MoveDown"}, {'6', "MoveRight"}}}
    };

public:
    std::vector<std::string> getPresetNames() {
        std::vector<std::string> names_list;
        for (const auto& item : key_presets) {
            names_list.push_back(item.first);
        }
        return names_list;
    }
    
    bool checkPreset(const std::string& preset_name) {
        return key_presets.find(preset_name) != key_presets.end();
    }
    
    std::map<char, std::string> getKeyMap(const std::string& preset_name) {
        auto it = key_presets.find(preset_name);
        if (it != key_presets.end()) {
            return it->second;
        }
        return {};
    }
};

class GameConfig {
private:
    int total_rounds;
    int ai_players;
    GameModeType game_type;
    
    bool checkRounds(int rounds) { 
        return rounds >= 1 && rounds <= 50;
    }
    
    bool checkAIPlayers(int ai_count) { 
        return ai_count >= 0 && ai_count <= 6;
    }

public:
    GameConfig() : total_rounds(3), ai_players(2), game_type(REGULAR) {}
    
    OperationResult changeRounds(int rounds) {
        if (!checkRounds(rounds)) {
            return OperationResult(BAD_VALUE, "Rounds should be 1 to 50");
        }
        total_rounds = rounds;
        return OperationResult();
    }
    
    OperationResult changeAIPlayers(int ai_count) {
        if (!checkAIPlayers(ai_count)) {
            return OperationResult(BAD_VALUE, "AI players should be 0 to 6");
        }
        ai_players = ai_count;
        return OperationResult();
    }
    
    void setGameType(GameModeType mode) { 
        game_type = mode; 
        if (mode == FAST) {
            total_rounds = 1;
        }
    }
    
    int getRounds() const { return total_rounds; }
    int getAIPlayers() const { return ai_players; }
    GameModeType getGameType() const { return game_type; }
};

class PlayerConfig {
private:
    std::string user_name;
    std::string keyboard_preset;
    std::map<char, std::string> custom_keybinds;
    ColorPlayer user_color;
    int map_dimension;
    
    KeyboardManager key_manager;
    
    bool checkMapSize(int size) { 
        return size >= 10 && size <= 60;
    }
    
    bool checkColorAllowed(ColorPlayer color) { 
        return color != COLOR_YELLOW;
    }

public:
    PlayerConfig() : user_name("Player1"), keyboard_preset("WASD_KEYS"), 
                    user_color(COLOR_RED), map_dimension(20) {}
    
    OperationResult changeUserName(const std::string& name) {
        if (name.empty() || name.length() > 25) {
            return OperationResult(BAD_VALUE, "Name must be 1-25 characters");
        }
        user_name = name;
        return OperationResult();
    }
    
    OperationResult changeKeyboardPreset(const std::string& preset_name) {
        if (!key_manager.checkPreset(preset_name)) {
            return OperationResult(BAD_VALUE, "Keyboard preset not available");
        }
        keyboard_preset = preset_name;
        return OperationResult();
    }
    
    OperationResult changeCustomKeys(const std::map<char, std::string>& key_mapping) {
        std::map<char, std::string> temp_check;
        for (const auto& key_pair : key_mapping) {
            if (temp_check.find(key_pair.first) != temp_check.end()) {
                return OperationResult(DATA_CONFLICT, "Duplicate key assignments");
            }
            temp_check[key_pair.first] = key_pair.second;
        }
        custom_keybinds = key_mapping;
        return OperationResult();
    }
    
    OperationResult changePlayerColor(ColorPlayer color) {
        if (!checkColorAllowed(color)) {
            return OperationResult(COLOR_USED, "Yellow color not allowed for players");
        }
        user_color = color;
        return OperationResult();
    }
    
    OperationResult changeMapSize(int size) {
        if (!checkMapSize(size)) {
            return OperationResult(BAD_VALUE, "Map size should be 10 to 60");
        }
        map_dimension = size;
        return OperationResult();
    }
    
    std::string getName() const { return user_name; }
    std::string getKeyboardPreset() const { return keyboard_preset; }
    std::map<char, std::string> getCustomKeys() const { return custom_keybinds; }
    ColorPlayer getColor() const { return user_color; }
    int getMapSize() const { return map_dimension; }
};

class GlobalGameState {
private:
    static GlobalGameState* single_instance;
    static std::mutex instance_lock;
    
    GameConfig current_game_config;
    PlayerConfig current_player_config;
    ScreenState current_screen;
    bool game_active;
    EventSystem event_system;
    
    GlobalGameState() : current_screen(SCREEN_MAIN), game_active(false) {}

public:
    GlobalGameState(const GlobalGameState&) = delete;
    GlobalGameState& operator=(const GlobalGameState&) = delete;
    
    static GlobalGameState& getGlobalState() {
        std::lock_guard<std::mutex> lock(instance_lock);
        if (single_instance == nullptr) {
            single_instance = new GlobalGameState();
        }
        return *single_instance;
    }
    
    EventSystem& getEventSystem() { return event_system; }
    
    GameConfig& getGameConfig() { return current_game_config; }
    void updateGameConfig(const GameConfig& config) { 
        current_game_config = config; 
        event_system.sendEvent("config_updated", {true, "Game config changed"});
    }
    
    PlayerConfig& getPlayerConfig() { return current_player_config; }
    void updatePlayerConfig(const PlayerConfig& config) { 
        current_player_config = config; 
        event_system.sendEvent("config_updated", {true, "Player config changed"});
    }
    
    ScreenState getScreen() const { return current_screen; }
    void switchScreen(ScreenState new_screen) { 
        current_screen = new_screen; 
        event_system.sendEvent("screen_changed", {true, "Screen switched"});
    }
    
    bool isGameActive() const { return game_active; }
    
    void registerEventListener(const std::string& event_name, std::function<void(const EventData&)> handler) {
        event_system.addListener(event_name, handler);
    }
    
    OperationResult beginGameSession() {
        auto round_check = current_game_config.changeRounds(current_game_config.getRounds());
        if (!round_check.good()) return round_check;
        
        auto map_check = current_player_config.changeMapSize(current_player_config.getMapSize());
        if (!map_check.good()) return map_check;
        
        if (current_player_config.getMapSize() < 12 && current_game_config.getAIPlayers() > 4) {
            return OperationResult(DATA_CONFLICT, "Map too small for many AI players");
        }
        
        game_active = true;
        switchScreen(SCREEN_PLAY);
        event_system.sendEvent("game_begun", {true, "Game started"});
        return OperationResult();
    }
    
    void stopGameSession() {
        game_active = false;
        switchScreen(SCREEN_MAIN);
    }
    
    OperationResult storeSettingsToFile(const std::string& filename) {
        try {
            std::ofstream output_file(filename);
            if (!output_file.is_open()) {
                return OperationResult(FILE_PROBLEM, "Cannot write to file");
            }
            
            output_file << "GameSettings:" << std::endl;
            output_file << "Rounds: " << current_game_config.getRounds() << std::endl;
            output_file << "AI Players: " << current_game_config.getAIPlayers() << std::endl;
            output_file << "Game Mode: " << current_game_config.getGameType() << std::endl;
            
            output_file << "PlayerSettings:" << std::endl;
            output_file << "Name: " << current_player_config.getName() << std::endl;
            output_file << "Keyboard: " << current_player_config.getKeyboardPreset() << std::endl;
            output_file << "Color: " << current_player_config.getColor() << std::endl;
            output_file << "Map Size: " << current_player_config.getMapSize() << std::endl;
            
            output_file.close();
            
            event_system.sendEvent("settings_stored", {true, "Settings saved"});
            return OperationResult();
            
        } catch (const std::exception& ex) {
            return OperationResult(FILE_PROBLEM, std::string("Save error: ") + ex.what());
        }
    }
    
    OperationResult loadSettingsFromFile(const std::string& filename) {
        try {
            std::ifstream input_file(filename);
            if (!input_file.is_open()) {
                return OperationResult(FILE_PROBLEM, "Cannot read from file");
            }
            
            std::string line;
            while (std::getline(input_file, line)) {
            }
            
            input_file.close();
            
            event_system.sendEvent("settings_loaded", {true, "Settings loaded"});
            return OperationResult();
            
        } catch (const std::exception& ex) {
            return OperationResult(FILE_PROBLEM, std::string("Load error: ") + ex.what());
        }
    }
};

GlobalGameState* GlobalGameState::single_instance = nullptr;
std::mutex GlobalGameState::instance_lock;

class StartScreenLogic {
private:
    GlobalGameState& game_state;

public:
    StartScreenLogic() : game_state(GlobalGameState::getGlobalState()) {}
    
    std::vector<int> getPossibleRounds() {
        return {1, 2, 3, 5, 7, 10, 15};
    }
    
    OperationResult processRoundChoice(int rounds) {
        return game_state.getGameConfig().changeRounds(rounds);
    }
    
    OperationResult processAIChoice(int ai_count) {
        return game_state.getGameConfig().changeAIPlayers(ai_count);
    }
    
    void setGameMode(GameModeType mode) {
        game_state.getGameConfig().setGameType(mode);
    }
    
    OperationResult launchGame() {
        return game_state.beginGameSession();
    }
};

class SettingsScreenLogic {
private:
    GlobalGameState& game_state;
    KeyboardManager key_manager;

public:
    SettingsScreenLogic() : game_state(GlobalGameState::getGlobalState()) {}
    
    std::vector<std::string> getKeyboardOptions() {
        return key_manager.getPresetNames();
    }
    
    OperationResult selectKeyboardLayout(const std::string& layout_name) {
        return game_state.getPlayerConfig().changeKeyboardPreset(layout_name);
    }
    
    OperationResult setCustomKeybinds(const std::map<char, std::string>& key_map) {
        return game_state.getPlayerConfig().changeCustomKeys(key_map);
    }
    
    OperationResult updatePlayerName(const std::string& new_name) {
        return game_state.getPlayerConfig().changeUserName(new_name);
    }
    
    OperationResult updatePlayerColor(ColorPlayer new_color) {
        return game_state.getPlayerConfig().changePlayerColor(new_color);
    }
    
    OperationResult updateMapSize(int new_size) {
        return game_state.getPlayerConfig().changeMapSize(new_size);
    }
    
    OperationResult storeAllSettings() {
        return game_state.storeSettingsToFile("game_settings.txt");
    }
    
    OperationResult loadAllSettings() {
        return game_state.loadSettingsFromFile("game_settings.txt");
    }
};

class AboutScreenLogic {
public:
    struct CreatorData {
        std::string person_name;
        std::string person_role;
        std::string contact_info;
    };
    
    std::vector<CreatorData> getTeamInfo() {
        return {
            {"Alex Developer", "Game Programming", "alex@game.com"},
            {"Maria Designer", "Graphics & UI", "maria@game.com"},
            {"John Tester", "Quality Assurance", "john@game.com"}
        };
    }
};

class ExitScreenLogic {
private:
    GlobalGameState& game_state;

public:
    ExitScreenLogic() : game_state(GlobalGameState::getGlobalState()) {}
    
    void confirmExit() {
        game_state.stopGameSession();
        game_state.registerEventListener("exit_confirmed", [](const EventData& data) {});
    }
    
    void cancelExit() {
        game_state.switchScreen(SCREEN_MAIN);
        game_state.registerEventListener("exit_cancelled", [](const EventData& data) {});
    }
};

class GameInterface {
private:
    StartScreenLogic start_logic;
    SettingsScreenLogic settings_logic;
    AboutScreenLogic about_logic;
    ExitScreenLogic exit_logic;
    GlobalGameState& game_state;

public:
    GameInterface() : game_state(GlobalGameState::getGlobalState()) {}
    
    void displayStartScreen() {
        game_state.switchScreen(SCREEN_START);
    }
    
    void displaySettingsScreen() {
        game_state.switchScreen(SCREEN_SETTINGS);
    }
    
    void displayAboutScreen() {
        game_state.switchScreen(SCREEN_ABOUT);
    }
    
    void displayExitScreen() {
        game_state.switchScreen(SCREEN_EXIT);
    }
    
    OperationResult executeGameStart(int rounds, int ai_count, GameModeType mode) {
        auto result = start_logic.processRoundChoice(rounds);
        if (!result.good()) return result;
        
        result = start_logic.processAIChoice(ai_count);
        if (!result.good()) return result;
        
        start_logic.setGameMode(mode);
        return start_logic.launchGame();
    }
    
    OperationResult executeSettingsSave(const std::string& name, ColorPlayer color, int map_size) {
        auto result = settings_logic.updatePlayerName(name);
        if (!result.good()) return result;
        
        result = settings_logic.updatePlayerColor(color);
        if (!result.good()) return result;
        
        result = settings_logic.updateMapSize(map_size);
        if (!result.good()) return result;
        
        return settings_logic.storeAllSettings();
    }
};

void runTests() {
    std::cout << "=== ТЕСТИРОВАНИЕ СИСТЕМЫ ===" << std::endl;
    
    int passed_tests = 0;
    int total_tests = 0;
    
    // Тест 1: GameConfig
    {
        total_tests++;
        GameConfig config;
        auto result1 = config.changeRounds(5);
        auto result2 = config.changeAIPlayers(3);
        
        if (result1.good() && result2.good() && config.getRounds() == 5 && config.getAIPlayers() == 3) {
            std::cout << "✓ GameConfig работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ GameConfig не работает" << std::endl;
        }
    }
    
    // Тест 2: PlayerConfig
    {
        total_tests++;
        PlayerConfig player;
        auto result1 = player.changeUserName("TestPlayer");
        auto result2 = player.changePlayerColor(COLOR_BLUE);
        auto result3 = player.changeMapSize(30);
        
        if (result1.good() && result2.good() && result3.good() && 
            player.getName() == "TestPlayer" && player.getColor() == COLOR_BLUE && player.getMapSize() == 30) {
            std::cout << "✓ PlayerConfig работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ PlayerConfig не работает" << std::endl;
        }
    }
    
    // Тест 3: Валидация PlayerConfig
    {
        total_tests++;
        PlayerConfig player;
        auto bad_name = player.changeUserName("");
        auto bad_color = player.changePlayerColor(COLOR_YELLOW);
        auto bad_size = player.changeMapSize(100);
        
        if (!bad_name.good() && !bad_color.good() && !bad_size.good()) {
            std::cout << "✓ Валидация PlayerConfig работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ Валидация PlayerConfig не работает" << std::endl;
        }
    }
    
    // Тест 4: KeyboardManager
    {
        total_tests++;
        KeyboardManager kb;
        auto layouts = kb.getPresetNames();
        bool has_wasd = std::find(layouts.begin(), layouts.end(), "WASD_KEYS") != layouts.end();
        bool check_preset = kb.checkPreset("WASD_KEYS");
        
        if (has_wasd && check_preset && !layouts.empty()) {
            std::cout << "✓ KeyboardManager работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ KeyboardManager не работает" << std::endl;
        }
    }
    
    // Тест 5: EventSystem
    {
        total_tests++;
        bool event_received = false;
        EventSystem events;
        
        events.addListener("test_event", [&](const EventData& data) {
            event_received = true;
        });
        
        events.sendEvent("test_event", {true, "Test message"});
        
        if (event_received) {
            std::cout << "✓ EventSystem работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ EventSystem не работает" << std::endl;
        }
    }
    
    // Тест 6: GlobalGameState Singleton
    {
        total_tests++;
        auto& state1 = GlobalGameState::getGlobalState();
        auto& state2 = GlobalGameState::getGlobalState();
        
        if (&state1 == &state2) {
            std::cout << "✓ GlobalGameState Singleton работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ GlobalGameState Singleton не работает" << std::endl;
        }
    }
    
    // Тест 7: StartScreenLogic
    {
        total_tests++;
        StartScreenLogic start;
        auto rounds = start.getPossibleRounds();
        auto result = start.processRoundChoice(10);
        
        if (!rounds.empty() && result.good()) {
            std::cout << "✓ StartScreenLogic работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ StartScreenLogic не работает" << std::endl;
        }
    }
    
    // Тест 8: SettingsScreenLogic
    {
        total_tests++;
        SettingsScreenLogic settings;
        auto layouts = settings.getKeyboardOptions();
        auto result = settings.updatePlayerName("SettingsTest");
        
        if (!layouts.empty() && result.good()) {
            std::cout << "✓ SettingsScreenLogic работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ SettingsScreenLogic не работает" << std::endl;
        }
    }
    
    // Тест 9: AboutScreenLogic
    {
        total_tests++;
        AboutScreenLogic about;
        auto team = about.getTeamInfo();
        
        if (!team.empty() && team[0].person_name == "Alex Developer") {
            std::cout << "✓ AboutScreenLogic работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ AboutScreenLogic не работает" << std::endl;
        }
    }
    
    // Тест 10: GameInterface
    {
        total_tests++;
        GameInterface game;
        game.displayStartScreen();
        auto& state = GlobalGameState::getGlobalState();
        
        if (state.getScreen() == SCREEN_START) {
            std::cout << "✓ GameInterface работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ GameInterface не работает" << std::endl;
        }
    }
    
    // Тест 11: Сохранение настроек
    {
        total_tests++;
        GlobalGameState& state = GlobalGameState::getGlobalState();
        auto result = state.storeSettingsToFile("test_settings.txt");
        
        if (result.good()) {
            std::cout << "✓ Сохранение настроек работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ Сохранение настроек не работает" << std::endl;
        }
    }
    
    // Тест 12: Запуск игры
    {
        total_tests++;
        GlobalGameState& state = GlobalGameState::getGlobalState();
        state.getGameConfig().changeRounds(3);
        state.getPlayerConfig().changeMapSize(20);
        auto result = state.beginGameSession();
        
        if (result.good() && state.isGameActive()) {
            std::cout << "✓ Запуск игры работает корректно" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ Запуск игры не работает" << std::endl;
        }
    }
    
    std::cout << "\n=== РЕЗУЛЬТАТЫ ТЕСТИРОВАНИЯ ===" << std::endl;
    std::cout << "Пройдено тестов: " << passed_tests << " из " << total_tests << std::endl;
    
    if (passed_tests == total_tests) {
        std::cout << "ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!" << std::endl;
    } else {
        std::cout << "ЕСТЬ ПРОБЛЕМЫ В РАБОТЕ СИСТЕМЫ" << std::endl;
    }
}

int main() {
    runTests();
    return 0;
}
