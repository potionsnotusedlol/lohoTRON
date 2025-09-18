#pragma once
#include <string>
#include <vector>
#include <array>
#include <cstdint>

class GameConfig
{
public:
    // перечисление раскладок клавиатуры
    enum class Layout : std::uint8_t
    {
        WASD,
        Arrows,
        IJKL,
        NumPad,
        Custom
    };
    //одно действие одна клавиша 
    struct KeyBind
    {
        std::string name;
        std::uint32_t key;
    };
    //профиль конченого автомата 
    struct BotProfile
    {
        std::string name;
        std::string color;
        std::uint8_t skill; // 0-100
    };

    GameConfig() noexcept;

    
    const std::string& playerName() const noexcept { return m_playerName; }
    const std::string& playerColor() const noexcept { return m_playerColor; }
    Layout layout() const noexcept { return m_layout; }
    const std::vector<KeyBind>& keyBinds() const noexcept { return m_keyBinds; }
    std::uint8_t bots() const noexcept { return m_bots; }
    std::uint8_t rounds() const noexcept { return m_rounds; }
    bool quickMatch() const noexcept { return m_quick; }
    std::uint16_t mapSize() const noexcept { return m_mapSize; }
    float masterVolume() const noexcept { return m_masterVol; }
    float sfxVolume() const noexcept { return m_sfxVol; }
    float musicVolume() const noexcept { return m_musicVol; }
    bool fullscreen() const noexcept { return m_fullscreen; }
    bool vsync() const noexcept { return m_vsync; }
    std::uint16_t resolutionX() const noexcept { return m_resX; }
    std::uint16_t resolutionY() const noexcept { return m_resY; }
    const std::vector<BotProfile>& botProfiles() const noexcept { return m_botProfiles; }

    bool setPlayerName(const std::string& name) noexcept;
    bool setPlayerColor(const std::string& hexRGB) noexcept;
    bool setLayout(Layout l) noexcept;
    bool setCustomKey(const std::string& action, std::uint32_t key) noexcept;
    bool setBots(std::uint8_t count) noexcept;
    bool setRounds(std::uint8_t count) noexcept;
    bool setQuickMatch(bool q) noexcept;
    bool setMapSize(std::uint16_t size) noexcept;
    bool setVolumes(float master, float sfx, float music) noexcept;
    bool setDisplay(bool fullscreen, bool vsync,
                    std::uint16_t width, std::uint16_t height) noexcept;

    /*сериализация для сохранения и загрузки из json*/
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;

    /* дефолт*/
    void resetToDefaults() noexcept;

private:
    std::string m_playerName;
    std::string m_playerColor;
    Layout m_layout;
    std::vector<KeyBind> m_keyBinds;
    std::vector<BotProfile> m_botProfiles;

    std::uint8_t m_bots;
    std::uint8_t m_rounds;
    bool m_quick;
    std::uint16_t m_mapSize;

    float m_masterVol;
    float m_sfxVol;
    float m_musicVol;

    bool m_fullscreen;
    bool m_vsync;
    std::uint16_t m_resX;
    std::uint16_t m_resY;

    void rebuildKeyBinds() noexcept;
    bool isColorReserved(const std::string& hex) const noexcept;
};