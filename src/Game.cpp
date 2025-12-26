#include <fstream>

#include "Game.hpp"
#include "MapLoader.hpp"

#pragma region Initialisation

void Game::Create(int argc, char** argv) {
    Renderer::LoadResources();
    Renderer::SetPlayerTheme(0u);

    initWindowIcon();

    loadSoundEffects();
    loadMusic();

    if (argc > 1) {
        const std::string filepath = argv[1u];

        const std::size_t dotPos = filepath.find_last_of('.');

        if (dotPos != std::string::npos) {
            const std::string extension = filepath.substr(dotPos + 1u);

            if (extension == "tas") {
                startCustomScript(filepath);
                return;
            }
        }
    }

    loadMap(1u, 1u);

    enterTitleScreen();
}

void Game::reload() {
    loadMap(player.Data.World, player.Data.Level);
}

void Game::setPaused(bool paused) {
    m_Paused = paused;

    if (paused) {
        audioPlayer.PauseAll();
        musicPlayer.Pause();
    }
}

#pragma region Events

void Game::ProcessEvents(const sf::Event& event) {
    if (m_BlackScreenTimer) {
        return;
    }

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        handleKeyPress(key->scancode);
    }
}

void Game::handleKeyPress(const sf::Keyboard::Scancode& key) {
    if (key == sf::Keyboard::Scancode::Enter) {
        if (m_OnTitleScreen) {
            if (m_DemoStartTimer) {
                exitTitleScreen();
            } else {
                restartDemo();
            }
        } else if (!m_PauseCooldown) {
            m_PauseCooldown = 44u;
            setPaused(!m_Paused);

            audioPlayer.Play(AudioPlayer::Pause);
        }
    }

    else if (key == sf::Keyboard::Scancode::G) {
        if (!m_World.cutscenePlaying()) {
            FireFlower({}).GrantPower(m_World);
        }
    }

    else if (key == sf::Keyboard::Scancode::LShift) {
        if (m_OnTitleScreen && m_DemoStartTimer) {
            m_World.TwoPlayerMode ^= true;
            m_DemoStartTimer = 24u; // reset
        }
    }

    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl)) {
        if (key == sf::Keyboard::Scancode::Z) {
            Rand::Reset();
            m_FrameruleTimer = 21u;

            reload();
            m_ScriptRecorder.StartRecording(player.Data.GetLevelPointer());
        }

        else if (key == sf::Keyboard::Scancode::X) {
            m_ScriptRecorder.StopRecording();
        }

        else if (key == sf::Keyboard::Scancode::S) {
            saveCustomScript();
        }

        else if (key == sf::Keyboard::Scancode::R) {
            reload();
        }

        else if (key == sf::Keyboard::Scancode::M) {
            audioPlayer.SetMuted(!audioPlayer.IsMuted());
            musicPlayer.SetMuted(!musicPlayer.IsMuted());
        }

        else if (key == sf::Keyboard::Scancode::Right || key == sf::Keyboard::Scancode::Left) {
            if (!m_OnTitleScreen) {
                if (key == sf::Keyboard::Scancode::Right) {
                    if (player.Data.GetLevelPointer() < 31u) {
                        if (++player.Data.Level == 5u) {
                            player.Data.Level = 1u;
                            ++player.Data.World;
                        }
                    }
                } else {
                    if (player.Data.GetLevelPointer()) {
                        if (--player.Data.Level == 0u) {
                            player.Data.Level = 4u;
                            --player.Data.World;
                        }
                    }
                }

                player.Data.Lives = std::max(uint8_t(1u), player.Data.Lives);

                loadMap(player.Data.World, player.Data.Level);
            }
        }
    }
}

#pragma region Framerules

void Game::tickFramerule() {
    if (--m_FrameruleTimer == 0u) {
        m_FrameruleTimer = FrameruleLength;

        if (m_OnTitleScreen && m_DemoStartTimer && --m_DemoStartTimer == 0u) {
            startDemoScript();
        }

        player.OnFramerule(m_World);
        m_World.OnFramerule();
    }
}

#pragma region Title Screen

void Game::enterTitleScreen() {
    m_OnTitleScreen = true;

    if (player.Data.Type == 1u) {
        player.Swap();
    }

    player.Data.World = 1u;
    player.Data.Level = 1u;

    player.m_Size = Player::Small;
    Renderer::SetPlayerTheme(0x00u);

    player.m_AcceptPlayerControls = false;

    m_DemoStartTimer = 24u;

    audioPlayer.SetMuted(true);
    musicPlayer.SetMuted(true);

    Renderer::SetGameTimeRendering(false);

    reload();
}

void Game::exitTitleScreen() {
    startBlackScreen(BlackScreenType::LevelTransition);

    player.ResetData();

    player.m_AcceptPlayerControls = true;
    player.m_TasMode = false;

    m_OnTitleScreen = false;

    audioPlayer.SetMuted(false);
    musicPlayer.SetMuted(false);

    musicPlayer.Stop();
    audioPlayer.StopAll();
}

void Game::restartDemo() {
    enterTitleScreen();

    pauseGameFor(21u);

    m_ScriptPlayer.Stop();

    Renderer::ResetAnimations();
}

#pragma region Black Screen

void Game::startBlackScreen(BlackScreenType type) {
    m_BlackScreenType = type;

    musicPlayer.Stop();

    pauseGameFor(5u);

    if (type == BlackScreenType::LevelTransition) {
        Renderer::SetBackgroundTheme(0u, 2u, 2u);
    }

    Renderer::ResetAnimations();
    Renderer::SetGameTimeRendering(false);

    if (type == BlackScreenType::GameOver) {
        m_BlackScreenTimer = 252u;

        musicPlayer.Play(MusicPlayer::GameOver);

        if (m_World.getGameTime() == 0u) {
            m_BlackScreenType = BlackScreenType::TimeUp;
        }
    } else {
        m_BlackScreenTimer = 144u;
    }
}

void Game::stopBlackScreen() {
    pauseGameFor(13u);

    if (m_BlackScreenType == BlackScreenType::GameOver) {
        if (m_World.TwoPlayerMode && player.m_SecondPlayerData.Lives) {
            player.Swap();
            startBlackScreen(BlackScreenType::LevelTransition);
        } else {
            enterTitleScreen();
        }
    } else if (m_BlackScreenType == BlackScreenType::LevelTransition) {
        reload();
    }
}

void Game::temporaryBlackScreen(uint8_t time) {
    reload();

    musicPlayer.Stop();

    pauseGameFor(time);

    m_World.StartThemeMusic();
}

void Game::pauseGameFor(uint8_t time) {
    while (true) {
        m_Window.clear();
        m_Window.display();

        if (--time == 0u) {
            break;
        }
    }
}

#pragma region Update

void Game::Update() {
    tickFramerule();

    if (m_PauseCooldown) {
        if (--m_PauseCooldown == 0u && !m_Paused) {
            audioPlayer.ResumeAll(44u);
            musicPlayer.Resume(44u);
        }
    }

    if (m_Paused) {
        return;
    }

    if (m_BlackScreenTimer) {
        if (--m_BlackScreenTimer == 0u) {
            stopBlackScreen();
        } else if (m_BlackScreenType == BlackScreenType::TimeUp && m_BlackScreenTimer == 150u) {
            m_BlackScreenType = BlackScreenType::GameOver;
        }

        return;
    }

    if (player.m_AcceptPlayerControls || m_OnTitleScreen) {
        m_ScriptPlayer.Update();
        m_ScriptRecorder.Update();
    }

    if (m_OnTitleScreen && m_ScriptPlayer.Done && !m_DemoStartTimer) {
        return restartDemo();
    }

    player.Update(m_World);
    m_World.Update();

    if (m_World.reloadRequired()) {
        if (player.Data.Lives == 0u) {
            startBlackScreen(BlackScreenType::GameOver);
        } else if (m_World.newLevel()) {
            startBlackScreen(BlackScreenType::LevelTransition);
        } else {
            temporaryBlackScreen(21u);
            Renderer::ResetAnimations();
        }

        return;
    }

    MapLoader::Update(m_World);

    Renderer::Animate();
}

#pragma region Map

void Game::loadMap(uint8_t world, uint8_t level) {
    player.Data.World = world;
    player.Data.Level = level;

    const uint8_t loadlevel = MapLoader::GetIfDuplicate(world, level);
    MapLoader::NewLevel(m_World, PlayerData::GetLevelPointer(loadlevel, level));

    m_World.StartThemeMusic();
}

#pragma region Rendering

sf::View Game::generateCameraView() const {
    return sf::View(sf::FloatRect(sf::Vector2f(m_World.CameraPosition, 0.f), sf::Vector2f(gbl::Width, gbl::Height)));
}

void Game::renderUi() {
    // set the camera position
    m_Window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {gbl::Width, gbl::Height})));

    // render
    Renderer::RenderUi(m_Window, m_World, m_OnTitleScreen);
}

void Game::renderGame() {
    // set the camera position
    m_Window.setView(generateCameraView());

    // render everything other than ui
    Renderer::RenderGame(m_Window, m_World);
}

void Game::Render() {
    if (m_BlackScreenTimer) {
        m_Window.clear();

        if (m_BlackScreenType != BlackScreenType::None) {
            if (m_BlackScreenType == BlackScreenType::LevelTransition) {
                Renderer::RenderBlackScreen_LevelTransition(m_Window);
            } else if (m_BlackScreenType == BlackScreenType::TimeUp) {
                Renderer::RenderBlackScreen_TimeUp(m_Window, m_World.TwoPlayerMode);
            } else /* if (m_BlackScreenType == BlackScreenType::GameOver) */ {
                Renderer::RenderBlackScreen_GameOver(m_Window, m_World.TwoPlayerMode);
            }

            renderUi();
        }
    } else {
        m_Window.clear(Renderer::BackgroundColor);

        renderUi();

        renderGame();

        if (m_OnTitleScreen && MapLoader::GetCurrentPage() <= 3u && m_World.CameraPosition < 256.f) {
            Renderer::RenderTitleScreen(m_Window, m_Highscore, m_World.TwoPlayerMode);
        }
    }

    m_Window.display();
}

#pragma region Utils

void Game::initWindowIcon() {
    sf::RenderTexture renderTexture(sf::Vector2u(16u, 16u));

    renderTexture.clear(sf::Color::Transparent);

    const sf::Vector2f oldPos = player.Position;

    player.Position = sf::Vector2f(0.f, -16.f);
    Renderer::RenderPlayer(renderTexture);

    player.Position = oldPos;

    renderTexture.display();

    m_Window.setIcon(renderTexture.getTexture().copyToImage());
}

sf::Image Game::GetScreenshot() const noexcept {
    constexpr uint8_t Scale = 4u;
    sf::RenderTexture renderTexture(sf::Vector2u(gbl::Width * Scale, gbl::Height * Scale));

    renderTexture.clear(Renderer::BackgroundColor);

    renderTexture.setView(sf::View(sf::FloatRect({0.f, 0.f}, {gbl::Width, gbl::Height})));

    Renderer::RenderUi(renderTexture, m_World, m_OnTitleScreen);

    renderTexture.setView(generateCameraView());

    Renderer::RenderGame(renderTexture, m_World);

    if (m_OnTitleScreen && MapLoader::GetCurrentPage() <= 3u && m_World.CameraPosition < 256.f) {
        Renderer::RenderTitleScreen(renderTexture, m_Highscore, m_World.TwoPlayerMode);
    }

    renderTexture.display();

    return renderTexture.getTexture().copyToImage();
}

void Game::loadSoundEffects() {
    const std::string basicPath = "Resources/Sfx/";
    const std::string extension = ".wav";

    for (uint8_t i = 0u; i < AudioPlayer::Count; ++i) {
        audioPlayer.LoadFromFile(i, basicPath + std::to_string(i) + extension);
    }
}

void Game::loadMusic() {
    const std::string basicPath = "Resources/Music/";
    const std::string extension = ".mp3";

    for (uint8_t i = 0u; i < MusicPlayer::Count - 1; ++i) {
        musicPlayer.LoadFromFile(i, basicPath + std::to_string(i) + extension);
    }
}

#pragma region Tas

void Game::startDemoScript() {
    std::vector<DemoInput> DemoScript = {
        {0x01u, 0x9Bu}, {0x80u, 0x10u}, {0x02u, 0x18u}, {0x81u, 0x05u},
        {0x41u, 0x2Cu}, {0x80u, 0x20u}, {0x01u, 0x24u}, {0x42u, 0x15u},
        {0xC2u, 0x5Au}, {0x02u, 0x10u}, {0x80u, 0x20u}, {0x41u, 0x28u},
        {0xC1u, 0x30u}, {0x41u, 0x20u}, {0xC1u, 0x10u}, {0x01u, 0x80u},
        {0xC1u, 0x20u}, {0x01u, 0x30u}, {0x02u, 0x30u}, {0x80u, 0x01u},
        {0x00u, 0xFFu}
    };

    m_ScriptPlayer.Start(DemoScript);
}

void Game::startCustomScript(const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);

    uint8_t level = 0;
    inFile.read(reinterpret_cast<char*>(&level), sizeof(level));

    loadMap((level >> 2u) + 1u, (level & 0x03u) + 1u);

    uint32_t count = 0;
    inFile.read(reinterpret_cast<char*>(&count), sizeof(count));

    std::vector<DemoInput> inputs(count);
    for (uint32_t i = 0u; i < count; ++i) {
        uint8_t& inputBit = inputs[i].inputBits;
        uint8_t& inputDuration = inputs[i].duration;

        inFile.read(reinterpret_cast<char*>(&inputBit), sizeof(inputBit));
        inFile.read(reinterpret_cast<char*>(&inputDuration), sizeof(inputDuration));
    }

    m_ScriptPlayer.Start(inputs);
}

std::string Game::saveCustomScript() {
    if (m_ScriptRecorder.Recording() || m_ScriptRecorder.getScript().empty()) {
        return "Naf";
    }

    const std::string directory = "Scripts/";
    const std::string baseName = "Script";
    const std::string extension = ".tas";

    std::string filepath = directory + baseName + extension; {
        uint8_t count = 1u;

        while (std::filesystem::exists(filepath)) {
            filepath = directory + baseName + " " + std::to_string(++count) + extension;
        }
    }

    std::ofstream outFile(filepath, std::ios::binary);

    const uint8_t level = m_ScriptRecorder.GetInitialLevel();
    outFile.write(reinterpret_cast<const char*>(&level), sizeof(level));

    const uint32_t count = static_cast<uint32_t>(m_ScriptRecorder.getScript().size());
    outFile.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& input : m_ScriptRecorder.getScript()) {
        const uint8_t& inputBit = input.inputBits;
        const uint8_t& inputDuration = input.duration;

        outFile.write(reinterpret_cast<const char*>(&inputBit), sizeof(inputBit));
        outFile.write(reinterpret_cast<const char*>(&inputDuration), sizeof(inputDuration));
    }

    m_ScriptRecorder.Reset();

    return filepath;
}