#ifndef GAME_HPP
#define GAME_HPP

#include "SFML/Graphics.hpp"

#include "World.hpp"
#include "Renderer.hpp"
#include "Utils/ControllerScript.hpp"

class Game {
    static constexpr inline const uint8_t FrameruleLength = 21u;

private:
    enum class BlackScreenType : uint8_t {
        None,
        LevelTransition,
        TimeUp,
        GameOver
    };

    void tickFramerule();

    void enterTitleScreen();
    void exitTitleScreen();
    void restartDemo();

    void startBlackScreen(BlackScreenType type);
    void stopBlackScreen();
    void pauseGameFor(uint8_t time);

    void renderUi();
    void renderGame();

    void handleKeyPress(const sf::Keyboard::Scancode& key);

    void reload();

    void startDemoScript();
    void makeScriptFromTas(const std::string& filename);
    void startCustomScript(const std::string& filename);
    std::string saveCustomScript();

    void setPaused(bool paused);

    void initWindowIcon();

    void takeScreenshot();

    void loadSoundEffects();
    void loadMusic();

    void loadMap(uint8_t level, uint8_t stage);

    sf::View generateCameraView();

    World m_World;

    bool m_Paused = false;
    uint8_t m_PauseCooldown = 0u;

    sf::RenderWindow& m_Window;

    ScriptRecorder m_ScriptRecorder;
    ScriptPlayer m_ScriptPlayer;

    uint8_t m_BlackScreenTimer = 0u;
    BlackScreenType m_BlackScreenType = BlackScreenType::None;

    bool m_OnTitleScreen = false;
    uint8_t m_DemoStartTimer = 0u;

    uint8_t m_FrameruleTimer = FrameruleLength;

    uint24_t m_Highscore;

public:
    Game(sf::RenderWindow& window) : m_Window(window) {}

    void Create(int argc, char** argv);

    void Update();
    void Render();

    void ProcessEvents(const sf::Event& event);

    inline const bool& SoundDisabled() const noexcept {
        return m_OnTitleScreen;
    }
};

#endif // !GAME_HPP