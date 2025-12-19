#include "Engine.hpp"

#pragma region Initialisation

Engine::Engine(int argc, char** argv) {
    const float windowScale = 3.f;

    const unsigned int windowWidth = gbl::Width * windowScale;
    const unsigned int windowHeight = gbl::Height * windowScale;

    m_Window.create(sf::VideoMode(sf::Vector2u(windowWidth, windowHeight)), "Super Mario Bros", sf::Style::Default);
    
    m_Window.clear();
    m_Window.display();

    m_Window.setKeyRepeatEnabled(false);
    m_Window.setFramerateLimit(gbl::Fps);
    
    m_LastWindowSize = m_Window.getSize();

    m_Game.Create(argc, argv);

    Rand::Reset();
}

#pragma region Events

void Engine::ProcessEvents() {
    while (const std::optional<sf::Event> event = m_Window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            return m_Window.close();
        }
        
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->scancode == sf::Keyboard::Scancode::Escape) {
                return m_Window.close();
            }
            
            else if (key->scancode == sf::Keyboard::Scancode::Backslash) {
                if (m_Paused) {
                    update_internal();
                } else {
                    setPaused(true);
                }
            }
            
            else if (key->scancode == sf::Keyboard::Scancode::Insert) {
                setPaused(false);
            }

            else if (key->scancode == sf::Keyboard::Scancode::F2) {
                saveScreenshot();
            }
        }
        
        else if (const auto* key = event->getIf<sf::Event::KeyReleased>()) {
            if (key->scancode == sf::Keyboard::Scancode::Backslash) {
                if (m_StepPressTime) {
                    m_StepPressTime = 0u;

                    setPaused(true);
                }
            }
        }
        
        else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            handleWindowResize(std::move(resized->size));
        }

        if (!m_Paused) {
            m_Game.ProcessEvents(event.value());
        }
    }
}

#pragma region Update

void Engine::update_internal() {
    m_Game.Update();

    Rand::Update();
}

void Engine::saveScreenshot() {
    const std::string baseName = "Screenshots";
    const std::string extension = ".png";
    const std::string directory = "Screenshots/";

    if (!std::filesystem::exists(directory)) {
        std::filesystem::create_directory(directory);
    }

    std::string filepath = directory + baseName + extension;
    {
        uint8_t count = 1u;

        while (std::filesystem::exists(filepath)) {
            filepath = directory + baseName + " " + std::to_string(count) + extension;
            ++count;
        }
    }

    if (!m_Game.GetScreenshot().saveToFile(filepath)) {
        LOG_ERROR("Failed To Save Screenshot");
    }
}

void Engine::Update() {
    if (m_Paused) {
        if (m_Window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Backslash)) {
            if (++m_StepPressTime == 30u) {
                setPaused(false);
            }
        }
    } else {
        update_internal();
    }
}

void Engine::setPaused(bool paused) {
    m_Paused = paused;
    
    audioPlayer.SetMuted(paused || m_Game.SoundDisabled());
    musicPlayer.SetMuted(paused || m_Game.SoundDisabled());
}

void Engine::handleWindowResize(sf::Vector2u newSize) {
    if (
        (newSize.x != m_LastWindowSize.x && newSize.y == m_LastWindowSize.y) ||
        (newSize.y == m_LastWindowSize.y || newSize.x != m_LastWindowSize.x) && newSize.x < newSize.y
    ) {
        unsigned int maxWidth = sf::VideoMode::getDesktopMode().size.y * gbl::Width / gbl::Height;

        if (newSize.x > maxWidth) {
            newSize.x = maxWidth;
            m_Window.setPosition(sf::Vector2i(m_Window.getPosition().x, 0));
        }

        newSize.y = newSize.x * gbl::Height / gbl::Width;
    } else {
        newSize.x = newSize.y * gbl::Width / gbl::Height;
    }

    m_Window.setSize(newSize);
    m_LastWindowSize = std::move(newSize);
}

#pragma region Render

void Engine::Render() {
    m_Game.Render();
}