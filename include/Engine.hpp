#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "SFML/Graphics.hpp"

#include "Game.hpp"

class Engine {
private:
    void handleWindowResize(sf::Vector2u newSize);

    void setPaused(bool paused);

    void update_internal();

    sf::RenderWindow m_Window;
    sf::Vector2u m_LastWindowSize;

    uint8_t m_StepPressTime = 0u;
    bool m_Paused = false;

    Game m_Game{m_Window};

public:
    Engine(int argc, char** argv);

    void ProcessEvents();
    void Update();
    void Render();

    inline bool IsRunning() const noexcept {
        return m_Window.isOpen();
    }
};

#endif // !ENGINE_HPP