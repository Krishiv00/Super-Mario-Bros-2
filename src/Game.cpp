#include <fstream>

#include "Game.hpp"
#include "MapLoader.hpp"

#pragma region Initialisation

void Game::Create(int argc, char** argv) {
    Renderer::LoadResources();
    Renderer::SetPlayerTheme(0x00u);

    initWindowIcon();

    loadSoundEffects();
    loadMusic();

    if (argc > 1) {
        const std::string filepath = argv[1u];

        const std::size_t dotPos = filepath.find_last_of('.');

        if (dotPos != std::string::npos) {
            const std::string extension = filepath.substr(dotPos + 1u);

            if (extension == "txt") {
                makeScriptFromTas(filepath);
                return;
            } else if (extension == "tas") {
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

    else if (key == sf::Keyboard::Scancode::F2) {
        takeScreenshot();
    }

    else if (key == sf::Keyboard::Scancode::LControl) {
        if (player.isFiery() && !player.m_Frozen) {
            player.ShootFireball();
        }
    }

    else if (key == sf::Keyboard::Scancode::G) {
        if (!m_World.cutscenePlaying()) {
            FireFlower({}).GrantPower(m_World);
        }
    }

    else if (key == sf::Keyboard::Scancode::H) {
        startBlackScreen(BlackScreenType::LevelTransition);
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

    player.m_AcceptPlayerControls = false;
    m_World.TwoPlayerMode = false;

    m_DemoStartTimer = 24u;

    audioPlayer.SetMuted(true);
    musicPlayer.SetMuted(true);
}

void Game::exitTitleScreen() {
    startBlackScreen(BlackScreenType::LevelTransition);

    player.m_AcceptPlayerControls = true;
    player.m_TasMode = false;

    m_OnTitleScreen = false;

    audioPlayer.SetMuted(false);
    musicPlayer.SetMuted(false);

    musicPlayer.Stop();
    audioPlayer.StopAll();
}

void Game::restartDemo() {
    player = Player();

    pauseGameFor(24u);
    reload();
    enterTitleScreen();

    m_ScriptPlayer.Stop();

    Renderer::ResetAnimations();
}

#pragma region Black Screen

void Game::startBlackScreen(BlackScreenType type) {
    m_BlackScreenType = type;
    m_BlackScreenTimer = 200u;

    Renderer::ResetAnimations();
    Renderer::SetGameTimeRendering(false);

    musicPlayer.Stop();
}

void Game::stopBlackScreen() {
    // TODO: add logic
    Renderer::SetGameTimeRendering(true);

    if (m_BlackScreenType == BlackScreenType::GameOver) {
        player = Player();
        m_World = World();
        Renderer::SetPlayerTheme(0x00u); // change for luigi

        loadMap(1u, 1u);
    } else {
        loadMap(player.Data.World, player.Data.Level);
    }
}

void Game::pauseGameFor(uint8_t time) {
    m_BlackScreenType = BlackScreenType::None;
    m_BlackScreenTimer = time;

    musicPlayer.Stop();
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
        if (m_World.newLevel()) {
            startBlackScreen(BlackScreenType::LevelTransition);
        } else {
            pauseGameFor(24u);
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

sf::View Game::generateCameraView() {
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

        if (
            m_BlackScreenType != BlackScreenType::None &&
            m_BlackScreenTimer > 14u && m_BlackScreenTimer < 186u
        ) {
            if (m_BlackScreenType == BlackScreenType::LevelTransition) {
                Renderer::RenderBlackScreen_LevelTransition(m_Window, m_World);
            } else if (m_BlackScreenType == BlackScreenType::TimeUp) {
                Renderer::RenderBlackScreen_TimeUp(m_Window);
            } else /* if (m_BlackScreenType == BlackScreenType::GameOver) */ {
                Renderer::RenderBlackScreen_GameOver(m_Window);
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

    sf::Vector2f oldPos = player.Position;

    player.Position = sf::Vector2f(0.f, -16.f);
    Renderer::RenderPlayer(renderTexture);

    player.Position = oldPos;

    renderTexture.display();

    sf::Image icon = renderTexture.getTexture().copyToImage();
    m_Window.setIcon(icon);
}

void Game::takeScreenshot() {
    sf::RenderTexture renderTexture(sf::Vector2u(gbl::Width, gbl::Height));

    renderTexture.clear(Renderer::BackgroundColor);

    renderTexture.setView(generateCameraView());

    Renderer::RenderGame(renderTexture, m_World);

    renderTexture.display();

    sf::Image screenshot = renderTexture.getTexture().copyToImage();

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

    if (!screenshot.saveToFile(filepath)) {
        LOG_ERROR("Failed To Save Screenshot");
    }
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

void Game::makeScriptFromTas(const std::string& filename) {
    std::ifstream inFile(filename);
    std::string line;

    while (std::getline(inFile, line)) {
        if (line.size() >= 2 && line.substr(0, 2) == "//") {
            continue;
        }

        if (line.substr(0, 5) == "World") {
            player.Data.World = line[6] - '0';
            player.Data.Level = line[8] - '0';

            m_ScriptRecorder.StartRecording(player.Data.GetLevelPointer());
            continue;
        }

        if (line.size() < 6) {
            continue;
        }

        if (line[0] == '1') {
            player.m_SprintKeyHeld = Player::SprintBufferLength;
        }

        player.m_JumpKeyHeld = line[1u] == '1';
        player.m_LeftKeyHeld = line[2u] == '1';
        player.m_RightKeyHeld = line[3u] == '1';
        player.m_UpKeyHeld = line[4u] == '1';
        player.m_DownKeyHeld = line[5u] == '1';

        m_ScriptRecorder.Update();
    }

    inFile.close();

    m_ScriptRecorder.StopRecording();

    std::string script = saveCustomScript();

    startCustomScript(script);

    std::filesystem::remove(script);
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

    uint8_t level = m_ScriptRecorder.GetInitialLevel();
    outFile.write(reinterpret_cast<const char*>(&level), sizeof(level));

    uint32_t count = static_cast<uint32_t>(m_ScriptRecorder.getScript().size());
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