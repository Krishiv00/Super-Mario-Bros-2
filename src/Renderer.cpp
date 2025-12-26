#include <sstream>

#include "Renderer.hpp"

#pragma region Graphics

void Renderer::LoadResources() noexcept {
    /*
    const auto LoadIndexedTextureFromMemory = [](const uint8_t* data) -> sf::Texture {
        sf::Vector2u size = sf::Vector2u(data[0], data[1]) * 8u;

        unsigned int blocksPerRow = size.x / 4u;

        sf::Image reconstructed;
        reconstructed.resize(sf::Vector2u(size.x, size.y));

        for (unsigned int y = 0; y < size.y; ++y) {
            for (unsigned int x = 0; x < size.x; ++x) {
                unsigned int pixelInBlock = x % 4;
                unsigned int shift = (3 - pixelInBlock) * 2;

                uint8_t byte = data[y * blocksPerRow + x / 4 + 2u];
                uint8_t paletteIndex = (byte >> shift) & 0x03;
                uint8_t grayValue = paletteIndex * 64;

                reconstructed.setPixel(sf::Vector2u(x, y), sf::Color(grayValue, grayValue, grayValue, 255 * (bool)grayValue));
            }
        }

        sf::Texture texture;

        texture.setSmooth(false);

        if (!texture.loadFromImage(reconstructed)) {
            LOG_ERROR("Failed To Load Indexed Texture");
        }

        return texture;
    };

    const auto LoadBitTextureFromMemory = [](const uint8_t* data) -> sf::Texture {
        sf::Vector2u size = sf::Vector2u(data[0], data[1]) * 8u;

        unsigned int blocksPerRow = size.x / 8u;

        sf::Image reconstructed;
        reconstructed.resize(size);

        for (unsigned int y = 0; y < size.y; ++y) {
            for (unsigned int x = 0; x < size.x; ++x) {
                unsigned int blockX = x / 8;
                unsigned int pixelInBlock = x % 8;
                unsigned int bitShift = 7 - pixelInBlock;

                unsigned int byteIndex = y * blocksPerRow + blockX + 2u;
                uint8_t byte = data[byteIndex];

                bool isOn = (byte >> bitShift) & 1;
                reconstructed.setPixel(sf::Vector2u(x, y), sf::Color(255, 255, 255, 255 * isOn));
            }
        }

        sf::Texture texture;

        texture.setSmooth(false);

        if (!texture.loadFromImage(reconstructed)) {
            LOG_ERROR("Failed To Load Bit Texture");
        }

        return texture;
    };
    */

    const auto LoadTextureFromFile = [](const char* filename) -> sf::Texture {
        const std::string path = "Resources/Textures/";
        const std::string filepath = path + filename + ".png";

        sf::Texture texture;

        if (!texture.loadFromFile(filepath)) {
            LOG_ERROR("Failed To Load Texture: " << filename);
        }

        return texture;
    };

    /*
    constexpr uint8_t FontTextureData[] = {
        0u
    };

    constexpr uint8_t TilesetTextureData[] = {
        0u
    };

    constexpr uint8_t PlayerTextureData[] = {
        0u
    };

    constexpr uint8_t SpritesTextureData[] = {
        0u
    };

    constexpr uint8_t PowerupsTextureData[] = {
        0u
    };

    constexpr uint8_t UiCoinTextureData[] = {
        0u
    };

    constexpr uint8_t JumpSpringTexture[] = {
        0u
    };

    constexpr uint8_t EndOfLevelSpritesTexture[] = {
        0u
    };

    constexpr uint8_t FloateyNumsTexture[] = {
        0u
    };

    constexpr uint8_t MiscSpritesTexture[] = {
        0u
    };
    */

    s_FontTexture = LoadTextureFromFile("Font");
    s_TilesetTexture = LoadTextureFromFile("Tileset");
    s_PlayerTexture = LoadTextureFromFile("Player");
    s_SpritesTexture = LoadTextureFromFile("Enemies");
    s_PowerupsTexture = LoadTextureFromFile("Powerups");
    s_UiTexture = LoadTextureFromFile("Ui");
    s_JumpSpringTexture = LoadTextureFromFile("Jump Spring");
    s_FloateyNumsTexture = LoadTextureFromFile("Floatey Nums");
    s_MiscSpritesTexture = LoadTextureFromFile("Misc Sprites");
    s_BannerTexture = LoadTextureFromFile("Banner");

    /*
    s_FontTexture = LoadBitTextureFromMemory(FontTextureData);
    s_TilesetTexture = LoadIndexedTextureFromMemory(TilesetTextureData);
    s_PlayerTexture = LoadIndexedTextureFromMemory(PlayerTextureData);
    s_SpritesTexture = LoadIndexedTextureFromMemory(SpritesTextureData);
    s_PowerupsTexture = LoadIndexedTextureFromMemory(PowerupsTextureData);
    s_UiTexture = LoadIndexedTextureFromMemory(UiCoinTextureData);
    s_JumpSpringTexture = LoadIndexedTextureFromMemory(JumpSpringTexture);
    s_EndOfLevelSpritesTexture = LoadIndexedTextureFromMemory(EndOfLevelSpritesTexture);
    s_FloateyNumsTexture = LoadIndexedTextureFromMemory(FloateyNumsTexture);
    s_MiscSpritesTexture = LoadIndexedTextureFromMemory(MiscSpritesTexture);
    */

    if (!s_PaletteShader.loadFromFile("Resources/Shaders/Pallete.fsh", sf::Shader::Type::Fragment)) {
        LOG_ERROR("Failed To Load Pallete Shader");
    }

    if (!s_SpritePallete.resize(sf::Vector2u(4u, 4u))) {
        LOG_ERROR("Failed To Resize Sprite Pallete Texture");
    }
}

#pragma region Palletes

void Renderer::SetBackgroundTheme(const bool& skyColIndex, const uint8_t& folliageColsIndex, const uint8_t& bodyColsIndex) noexcept {
    constexpr std::array<uint8_t, 3u> BackgroundColorSets[] = {
        {0u, 0u, 0u},
        {93u, 150u, 255u}
    };

    constexpr std::array<std::array<uint8_t, 3u>, 3u> FolliageColorSets[] = {
        {{
            {231u, 0u, 89u}, {32u, 56u, 239u}, {255u, 117u, 182u}
            }},
        {{
            {130u, 211u, 16u}, {0u, 170u, 0u}, {0u, 0u, 0u}
            }},
        {{
            {130u, 211u, 16u}, {0u, 170u, 0u}, {0u, 69u, 0u}
            }},
        {{
            {255u, 255u, 255u}, {190u, 190u, 190u}, {117u, 117u, 117u}
            }},
        {{
            {255u, 154u, 56u}, {219u, 40u, 0u}, {0u, 0u, 0u}
            }},
    };

    constexpr std::array<std::array<uint8_t, 3u>, 12u> BodyColorSets[] = {
        {{
            {0u, 0u, 0u}, {170u, 243u, 190u}, {0u, 170u, 0u}, {0u, 0u, 0u},
        {0u, 0u, 0u}, {255u, 255u, 255u}, {32u, 56u, 239u}, {0u, 0u, 0u},
        {0u, 0u, 0u}, {255u, 154u, 56u}, {32u, 56u, 239u}, {0u, 0u, 0u}
            }},
        {{
            {0u, 0u, 0u}, {255u, 190u, 178u}, {203u, 77u, 12u}, {0u, 0u, 0u},
        {0u, 0u, 0u}, {255u, 255u, 255u}, {60u, 190u, 255u}, {0u, 0u, 0u},
        {0u, 0u, 0u}, {255u, 154u, 56u}, {203u, 77u, 12u}, {0u, 0u, 0u}
            }},
        {{
            {0u, 0u, 0u}, {158u, 255u, 243u}, {0u, 130u, 138u}, {0u, 0u, 0u},
        {0u, 0u, 0u}, {255u, 255u, 255u}, {60u, 190u, 255u}, {0u, 130u, 138u},
        {0u, 0u, 0u}, {255u, 154u, 56u}, {203u, 77u, 12u}, {0u, 130u, 138u}
            }},
        {{
            {0u, 0u, 0u}, {255u, 255u, 255u}, {190u, 190u, 190u}, {117u, 117u, 117u},
        {0u, 0u, 0u}, {255u, 255u, 255u}, {219u, 40u, 0u}, {117u, 117u, 117u},
        {0u, 0u, 0u}, {255u, 154u, 56u}, {203u, 77u, 12u}, {117u, 117u, 117u}
            }}
    };

    sf::Image img(sf::Vector2u(4u, 4u));

    {
        const auto& c = BackgroundColorSets[skyColIndex];
        img.setPixel(sf::Vector2u(0u, 0u), sf::Color(c[0u], c[1u], c[2u]));
    }

    for (uint8_t i = 0u; i < 3u; ++i) {
        const auto& c = FolliageColorSets[folliageColsIndex][i];
        img.setPixel(sf::Vector2u(i + 1u, 0u), sf::Color(c[0u], c[1u], c[2u]));
    }

    for (uint8_t i = 0u; i < 12u; ++i) {
        const auto& c = BodyColorSets[bodyColsIndex][i];
        img.setPixel(sf::Vector2u(i % 4u, i / 4u + 1u), sf::Color(c[0u], c[1u], c[2u]));
    }

    if (!s_BackgroundPallete.loadFromImage(img)) {
        LOG_ERROR("Failed To Load Background Pallete");
    }

    BackgroundColor = s_BackgroundPallete.copyToImage().getPixel(sf::Vector2u(0u, 0u));

    UpdatePalleteColors();
}

void Renderer::SetSpriteTheme(const uint8_t& theme, uint8_t endIndex) noexcept {
    constexpr std::array<std::array<uint8_t, 4u>, 12u> SpriteColorSets[] = {
        {{
            {0u, 0u, 0u}, {190u, 190u, 190u}, {255u, 255u, 255u}, {255u, 154u, 56u},
        {0u, 0u, 0u}, {219u, 40u, 0u}, {255u, 255u, 255u}, {255u, 154u, 56u},
        {0u, 0u, 0u}, {0u, 0u, 0u}, {255u, 255u, 255u}, {190u, 190u, 190u}
            }},
        {{
            {0u, 0u, 0u}, {0u, 170u, 0u}, {255u, 255u, 255u}, {255u, 154u, 56u},
        {0u, 0u, 0u}, {219u, 40u, 0u}, {255u, 255u, 255u}, {255u, 154u, 56u},
        {0u, 0u, 0u}, {0u, 0u, 0u}, {255u, 190u, 178u}, {203u, 77u, 12u}
            }},
        {{
            {0u, 0u, 0u}, {0u, 130u, 138u}, {255u, 190u, 178u}, {203u, 77u, 12u},
        {0u, 0u, 0u}, {219u, 40u, 0u}, {255u, 255u, 255u}, {255u, 154u, 56u},
        {0u, 0u, 0u}, {24u, 60u, 93u}, {158u, 255u, 243u}, {0u, 130u, 138u}
            }},
        {{
            {0u, 0u, 0u}, {0u, 130u, 138u}, {255u, 190u, 178u}, {203u, 77u, 12u},
        {0u, 0u, 0u}, {219u, 40u, 0u}, {255u, 255u, 255u}, {255u, 154u, 56u},
        {0u, 0u, 0u}, {117u, 117u, 117u}, {255u, 255u, 255u}, {190u, 190u, 190u}
            }}
    };

    sf::Image img = s_SpritePallete.copyToImage();

    for (uint8_t i = 0u; i < endIndex; ++i) {
        const auto& c = SpriteColorSets[theme][i];
        img.setPixel(sf::Vector2u(i % 4u, i / 4u + 1u), sf::Color(c[0u], c[1u], c[2u]));
    }

    if (!s_SpritePallete.loadFromImage(img)) {
        LOG_ERROR("Failed To Load Sprite Pallete");
    }
}

void Renderer::SetPlayerTheme(const uint8_t& theme) noexcept {
    constexpr std::array<std::array<uint8_t, 4u>, 12u> PlayerColorSets[] = {
        {{
            {0u, 0u, 0u}, {219u, 40u, 0u}, {255u, 154u, 56u}, {138u, 113u, 0u}
            }},
        {{
            {0u, 0u, 0u}, {255u, 255u, 255u}, {255u, 154u, 56u}, {0u, 170u, 0u}
            }},
        {{
            {0u, 0u, 0u}, {255u, 219u, 170u}, {255u, 154u, 56u}, {219u, 40u, 0u}
            }},
    };

    sf::Image img = s_SpritePallete.copyToImage();

    for (uint8_t i = 0u; i < 4u; ++i) {
        const auto& c = PlayerColorSets[theme][i];
        img.setPixel(sf::Vector2u(i, 0u), sf::Color(c[0u], c[1u], c[2u]));
    }

    if (!s_SpritePallete.loadFromImage(img)) {
        LOG_ERROR("Failed To Load Sprite Pallete");
    }
}

#pragma region Animation

void Renderer::UpdatePalleteColors() noexcept {
    constexpr sf::Color coinPalleteColors[] = {
        sf::Color(255u, 154u, 56u), sf::Color(203u, 77u, 12u), sf::Color(125u, 8u, 0u)
    };

    const sf::Color& currentColor = coinPalleteColors[s_BlinkAnimation];

    sf::Image img = s_BackgroundPallete.copyToImage();
    img.setPixel(sf::Vector2u(13u, 0u), currentColor);

    if (!s_BackgroundPallete.loadFromImage(img)) {
        LOG_ERROR("Failed To Load Image To Texture");
    }
}

void Renderer::Animate() noexcept {
    if (--s_BlinkAnimationTimer == 0) {
        s_BlinkAnimationTimer = AnimationTimerDuration;

        s_BlinkAnimation += s_BlinkDirection;

        if (s_BlinkAnimation == (s_BlinkDirection == 1 ? 2u : 0u)) {
            s_BlinkDirection *= -1;

            // animation has reset
            if (s_BlinkDirection == 1) {
                s_BlinkAnimationTimer = AnimationTimerDuration * 3u;
            }
        }

        UpdatePalleteColors();
    }

    if (--s_SpriteAnimationTimer == 0) {
        s_SpriteAnimationTimer = AnimationTimerDuration;
        s_EnemyAnimation ^= 1u;
    }
}

void Renderer::ResetAnimations() noexcept {
    s_BlinkAnimationTimer = AnimationTimerDuration;
    s_SpriteAnimationTimer = AnimationTimerDuration;

    s_BlinkDirection = 1;

    s_BlinkAnimation = 0u;
    s_EnemyAnimation = 0u;

    UpdatePalleteColors();
}

#pragma region Rendering

#if RENDER_HITBOXES
void Renderer::appendHitbox(sf::VertexArray& vertices, const sf::FloatRect& hitbox) noexcept {
    constexpr sf::Color color = sf::Color::Green;

    sf::Vertex topleft = sf::Vertex(hitbox.position, color);
    sf::Vertex topright = sf::Vertex({hitbox.position.x + hitbox.size.x, hitbox.position.y}, color);
    sf::Vertex bottomright = sf::Vertex(hitbox.position + hitbox.size, color);
    sf::Vertex bottomleft = sf::Vertex({hitbox.position.x, hitbox.position.y + hitbox.size.y}, color);

    vertices.append(topleft);
    vertices.append(topright);
    vertices.append(bottomright);

    vertices.append(topleft);
    vertices.append(bottomright);
    vertices.append(bottomleft);
}
#endif // RENDER_HITBOXES

void Renderer::SetGameTimeRendering(bool state) noexcept {
    s_RenderGameTime = state;
}

void Renderer::createVertices(const sf::Vector2f& position, const sf::Vector2f& texturePosition, const sf::Vector2f& size = sf::Vector2f(TileSize, TileSize), bool flipHorizontally = false, bool flipVertically = false) noexcept {
    sf::Vertex topleft = sf::Vertex(position, sf::Color::White, texturePosition);
    sf::Vertex topright = sf::Vertex({position.x + size.x, position.y}, sf::Color::White, {texturePosition.x + size.x, texturePosition.y});
    sf::Vertex bottomright = sf::Vertex(position + size, sf::Color::White, texturePosition + size);
    sf::Vertex bottomleft = sf::Vertex({position.x, position.y + size.y}, sf::Color::White, {texturePosition.x, texturePosition.y + size.y});

    if (flipVertically) {
        std::swap(topleft.position, bottomleft.position);
        std::swap(topright.position, bottomright.position);
    } else if (flipHorizontally) {
        std::swap(topleft.position, topright.position);
        std::swap(bottomleft.position, bottomright.position);
    }

    s_Vertices[0u] = topleft;
    s_Vertices[1u] = std::move(topright);
    s_Vertices[2u] = bottomright;

    s_Vertices[3u] = std::move(topleft);
    s_Vertices[4u] = std::move(bottomright);
    s_Vertices[5u] = std::move(bottomleft);
}

void Renderer::renderVertices(const sf::Texture& texture, const uint8_t& subPalleteIndex, sf::RenderTarget& target) noexcept {
    s_PaletteShader.setUniform("subPallete", 0.25f * static_cast<float>(subPalleteIndex));

    sf::RenderStates state(&texture);
    state.shader = &s_PaletteShader;

    target.draw(s_Vertices, 6u, sf::PrimitiveType::Triangles, state);
}

#pragma region Black Screen

void Renderer::RenderBlackScreen_LevelTransition(sf::RenderTarget& target) noexcept {
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);

    // world
    std::string worldText = "WORLD " + std::to_string(player.Data.World) + "-" + std::to_string(player.Data.Level);
    textAddString(worldText, sf::Vector2f(88.f, 80.f), vertices);

    // cross
    textAddChar('x', sf::Vector2f(120.f, 113.f), vertices);

    // lives
    textAddChar('0' + player.Data.Lives, sf::Vector2f(144.f, 112.f), vertices);

    textFlush(target, vertices);

    /* player sprite */ {
        s_PaletteShader.setUniform("pallete", s_SpritePallete);

        constexpr sf::Vector2f Position = sf::Vector2f(96.f, 105.f);
        constexpr sf::Vector2f TexturePos = sf::Vector2f(0.f, TileSize);

        const bool isFiery = player.isFiery();

        if (isFiery) {
            SetPlayerTheme(player.Data.Type);
        }

        createVertices(Position, TexturePos);
        renderVertices(s_PlayerTexture, player.SubPalleteIndex, target);

        if (isFiery) {
            SetPlayerTheme(0x02u);
        }
    }
}

void Renderer::RenderBlackScreen_TimeUp(sf::RenderTarget& target, bool twoPlayerMode) noexcept {
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);

    if (twoPlayerMode) {
        textAddString(player.Name(), sf::Vector2f(104.f, 112.f), vertices);
    }

    textAddString("TIME UP", sf::Vector2f(96.f, 128.f), vertices);

    textFlush(target, vertices);
}

void Renderer::RenderBlackScreen_GameOver(sf::RenderTarget& target, bool twoPlayerMode) noexcept {
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);

    if (twoPlayerMode) {
        textAddString(player.Name(), sf::Vector2f(104.f, 112.f), vertices);
    }

    textAddString("GAME OVER", sf::Vector2f(88.f, 128.f), vertices);

    textFlush(target, vertices);
}

#pragma region Player

void Renderer::RenderPlayer(sf::RenderTarget& target) noexcept {
    if (!player.isVisible()) {
        return;
    }

    const uint8_t row = (player.m_FireballThrowAnimation ? Player::Shooting : player.getState()) * 2u;
    const uint8_t col = (player.isBig() || player.m_FireballThrowAnimation) * 5u + player.m_AnimationFrame * !player.m_FireballThrowAnimation;

    const sf::Vector2f position = sf::Vector2f(player.xPosition(), player.yPosition());
    const sf::Vector2f texturePos = sf::Vector2f(static_cast<float>(col) * TileSize, static_cast<float>(row) * TileSize);

    s_PaletteShader.setUniform("pallete", s_SpritePallete);

    createVertices(position, texturePos, sf::Vector2f(TileSize, TileSize * 2.f), player.m_Direction == gbl::Direction::Left);
    renderVertices(s_PlayerTexture, player.SubPalleteIndex, target);

#if RENDER_HITBOXES
    sf::VertexArray hitboxes(sf::PrimitiveType::Triangles);

    appendHitbox(hitboxes, player.getHitbox());

    target.draw(hitboxes);
#endif // RENDER_HITBOXES
}

#pragma region Tile

void Renderer::render(sf::RenderTarget& target, const uint8_t& textureId, const uint8_t& subPalleteIndex, sf::Vector2f position) noexcept {
    const sf::Vector2f texturePos = sf::Vector2f((textureId - 1) * TileSize, 0.f);

    createVertices(position, texturePos);
    renderVertices(s_TilesetTexture, subPalleteIndex, target);
}

#pragma region Enemy

void Renderer::render(sf::RenderTarget& target, const Enemy& enemy, sf::VertexArray& hitboxes) noexcept {
    const sf::Vector2f texturePos = sf::Vector2f(
        enemy.m_Animate && s_EnemyAnimation, (enemy.m_Type & 0x7F) * 2u
    ) * TileSize;

    createVertices(enemy.Position, texturePos, sf::Vector2f(TileSize, TileSize * 2.f), enemy.m_Direction == 1);
    renderVertices(s_SpritesTexture, enemy.SubPalleteIndex, target);

#if RENDER_HITBOXES
    appendHitbox(hitboxes, enemy.getHitbox());
#endif // RENDER_HITBOXES
}

#pragma region NPC

void Renderer::render(sf::RenderTarget& target, const NPC& npc) noexcept {
    const sf::Vector2f texturePos = sf::Vector2f(npc.m_Animate, EnemyType::NPC * 2u) * TileSize;

    createVertices(npc.Position, texturePos, sf::Vector2f(TileSize, TileSize * 2.f));
    renderVertices(s_SpritesTexture, npc.SubPalleteIndex, target);
}

#pragma region Shell

void Renderer::render(sf::RenderTarget& target, const EnemyComponents::Shell& shell, sf::VertexArray& hitboxes) noexcept {
    sf::Vector2f position = shell.Position;

    const sf::Vector2f texturePos = sf::Vector2f(
        shell.m_Animate && s_EnemyAnimation, (shell.m_Type & 0x7F) * 2u
    ) * TileSize;

    if (shell.IsFlipped()) {
        position.y += TileSize;
    }

    createVertices(position, texturePos, sf::Vector2f(TileSize, TileSize * 2.f), shell.m_Direction == 1, shell.IsFlipped());
    renderVertices(s_SpritesTexture, shell.SubPalleteIndex, target);

#if RENDER_HITBOXES
    appendHitbox(hitboxes, shell.getHitbox());
#endif // RENDER_HITBOXES
}

#pragma region Firebar

void Renderer::render(sf::RenderTarget& target, const Firebar& firebar) noexcept {
    const sf::Vector2f texturePos = fireballTexturePos();

    const float angleRad = firebar.getAngle();
    const float angleSin = std::sin(angleRad);
    const float angleCos = std::cos(angleRad);

    for (uint8_t i = 0u; i < firebar.getSize(); ++i) {
        const float radius = i * TileSize * 0.5f;

        sf::Vector2f position = sf::Vector2f(
            firebar.Position.x + angleSin * radius,
            firebar.Position.y + angleCos * radius
        );

        createVertices(position, texturePos);
        renderVertices(s_SpritesTexture, firebar.SubPalleteIndex, target);
    }
}

#pragma region Powerup

void Renderer::render(sf::RenderTarget& target, const Powerup& powerup, sf::VertexArray& hitboxes) noexcept {
    if (powerup.m_Type == gbl::PowerupType::FireFlower) {
        const sf::Vector2f texturePos = sf::Vector2f((gbl::PowerupType::FireFlower + 1u) * TileSize, 0.f);

        createVertices(powerup.Position, texturePos);
        renderVertices(s_PowerupsTexture, 1u, target);
    }

    const sf::Vector2f texturePos = sf::Vector2f(powerup.m_Type * TileSize, 0.f);

    createVertices(powerup.Position, texturePos);
    renderVertices(s_PowerupsTexture, powerup.SubPalleteIndex, target);

#if RENDER_HITBOXES
    appendHitbox(hitboxes, powerup.m_Hitbox);
#endif // RENDER_HITBOXES
}

#pragma region Jump Spring

void Renderer::render(sf::RenderTarget& target, const JumpSpring& spring) noexcept {
    const sf::Vector2f texturePos = sf::Vector2f(spring.getCurrentStage() * TileSize, 0.f);

    createVertices(spring.Position, texturePos, sf::Vector2f(TileSize, TileSize * 1.5f));
    renderVertices(s_JumpSpringTexture, spring.SubPalleteIndex, target);
}

#pragma region Flag

void Renderer::render(sf::RenderTarget& target, const Flag& flag) noexcept {
    createVertices(flag.Position, sf::Vector2f(0.f, (EnemyType::Flag * 2u + 1u) * TileSize));
    renderVertices(s_SpritesTexture, flag.SubPalleteIndex, target);

    if (flag.m_FloateyNumType >= 0) {
        const sf::Vector2f position = sf::Vector2f(flag.Position.x + 21.f, flag.m_FloateyNumYPos);
        const sf::Vector2f texturePos = sf::Vector2f(flag.m_FloateyNumType * TileSize, 0.f);

        createVertices(position, texturePos);
        renderVertices(s_FloateyNumsTexture, flag.SubPalleteIndex, target);
    }
}

#pragma region Star Flag

void Renderer::render(sf::RenderTarget& target, const StarFlag& flag) noexcept {
    const sf::Vector2f texturePos = sf::Vector2f(1.f, EnemyType::Flag * 2u + 1u) * TileSize;

    createVertices(flag.Position, texturePos);
    renderVertices(s_SpritesTexture, flag.SubPalleteIndex, target);
}

#pragma region Axe

void Renderer::render(sf::RenderTarget& target, const Axe& axe) noexcept {
    const sf::Vector2f texturePos = sf::Vector2f(0.f, EnemyType::Axe * 2u * TileSize);

    s_PaletteShader.setUniform("pallete", s_BackgroundPallete);

    createVertices(axe.Position, texturePos, sf::Vector2f(TileSize, TileSize * 2.f));
    renderVertices(s_SpritesTexture, axe.SubPalleteIndex, target);

    s_PaletteShader.setUniform("pallete", s_SpritePallete);
}

#pragma region Lift

void Renderer::render(sf::RenderTarget& target, const Lift& lift, bool balanceLift) noexcept {
    if (balanceLift && lift.Position.y > 48.f) {
        s_PaletteShader.setUniform("pallete", s_BackgroundPallete);

        render(target, gbl::TextureId::Block::String_2, 1u, sf::Vector2f(lift.Position.x + 16.f, std::max(40.f, lift.Position.y - 16.f)));

        s_PaletteShader.setUniform("pallete", s_SpritePallete);
    }

    const sf::Vector2f texturePos = sf::Vector2f(lift.m_Animate, EnemyType::Lift * 2u + 1u) * TileSize;

    for (uint8_t i = 0u; i < lift.getSize(); ++i) {
        const sf::Vector2f position = sf::Vector2f(lift.Position.x + i * TileSize, lift.Position.y);

        createVertices(position, texturePos);
        renderVertices(s_SpritesTexture, lift.SubPalleteIndex, target);
    }
}

#pragma region Fireball

void Renderer::render(sf::RenderTarget& target, const Fireball& ball) noexcept {
    const sf::Vector2f texturePos = fireballTexturePos();

    createVertices(ball.Position, texturePos);
    renderVertices(s_SpritesTexture, ball.SubPalleteIndex, target);
}

#pragma region Misc Sprite

void Renderer::render(sf::RenderTarget& target, const DecorSprite& sprite) noexcept {
    const sf::Vector2f texturePos = sf::Vector2f(sprite.GetTextureIndex() * TileSize, 0.f);

    createVertices(sprite.Position, texturePos);
    renderVertices(s_MiscSpritesTexture, 2u, target);
}

#pragma region Floatey Num

void Renderer::render(sf::RenderTarget& target, const FloateyNum& floateyNum, uint8_t subPalleteIndex, float cameraPos) noexcept {
    const sf::Vector2f position = floateyNum.getPosition(cameraPos);
    const sf::Vector2f texturePos = sf::Vector2f(floateyNum.getType() * TileSize, 0.f);

    createVertices(position, texturePos);
    renderVertices(s_FloateyNumsTexture, subPalleteIndex, target);
}

#pragma region Death Animation

void Renderer::render(sf::RenderTarget& target, const DeathAnimation& animation) noexcept {
    const sf::Vector2f texturePos = sf::Vector2f(0.f, animation.getType() * 2u * TileSize);

    createVertices(animation.Position, texturePos, sf::Vector2f(TileSize, TileSize * 2.f), false, true);
    renderVertices(s_SpritesTexture, animation.SubPalleteIndex, target);
}

#pragma region Utils

bool Renderer::hiddenSlot(const Enemy* enemy) noexcept {
    return enemy->m_Type == EnemyType::PiranhaPlant;
}

bool Renderer::hiddenSlot(const Powerup* powerup) noexcept {
    return powerup->m_GetOutTimer;
}

std::string Renderer::intToStringFixedSize(unsigned int _int, uint8_t length) noexcept {
    std::stringstream ss;
    ss << std::setw(length) << std::setfill('0') << _int;
    return ss.str();
}

sf::Vector2f Renderer::fireballTexturePos() {
    return sf::Vector2f(s_EnemyAnimation, EnemyType::Firebar * 2u + (s_SpriteAnimationTimer <= 3u)) * TileSize;
}

#pragma region Tiles

void Renderer::renderTiles(sf::RenderTarget& target, const World& world) noexcept {
    s_PaletteShader.setUniform("pallete", s_BackgroundPallete);

    const unsigned int startCol = static_cast<unsigned int>((world.CameraPosition) / TileSize);
    const unsigned int endCol = (world.CameraPosition + gbl::Width) / TileSize;

    for (unsigned int x = startCol; x <= endCol; ++x) {
        unsigned int colIndex = x * 13u;

        float posX = static_cast<float>(x) * TileSize;

        for (uint8_t y = 0u; y < 13u; ++y) {
            if (const auto& block = world.m_Tiles[colIndex + y]) {
                if (const Components::Render* renderComponent = GetComponent(block.get(), const Components::Render)) {
                    float posY = static_cast<float>(y + 2u) * TileSize;
                    const sf::Vector2f position = sf::Vector2f(posX, posY);

                    if (
                        world.CurrentTheme == 0x00u &&
                        (renderComponent->TextureId == gbl::TextureId::Block::Coin || renderComponent->TextureId == gbl::TextureId::Block::SeaWeed)
                    ) {
                        render(target, gbl::TextureId::Block::Liquid_2, 2u, position);
                    }

                    render(target, renderComponent->TextureId, world.m_AttributeTable[colIndex + y], position);
                }
            }
        }
    }

    if (BouncingBlock* bouncingBlock = world.m_BouncingBlock.get()) {
        render(target, GetComponent(bouncingBlock->getBlock(), const Components::Render)->TextureId, bouncingBlock->SubPalleteIndex, bouncingBlock->getPosition());
    }

    // draw page borders
#if 0
    constexpr sf::Color lineColor = sf::Color::Green;

    const sf::Vertex lines[] = {
        sf::Vertex(sf::Vector2f(256.f, 0.f), lineColor),
        sf::Vertex(sf::Vector2f(256.f, gbl::Height), lineColor),

        sf::Vertex(sf::Vector2f(512.f, 0.f), lineColor),
        sf::Vertex(sf::Vector2f(512.f, gbl::Height), lineColor)
    };

    target.draw(lines, 4u, sf::PrimitiveType::Lines);
#endif
}

#pragma region Sprites

void Renderer::renderSprites(sf::RenderTarget& target, const World& world, bool drawHidden) noexcept {
    s_PaletteShader.setUniform("pallete", s_SpritePallete);

    sf::VertexArray hitboxes(sf::PrimitiveType::Triangles);

    for (const auto& slot : world.m_Sprites) {
        if (slot) {
            if (const Enemy* enemy = GetIf(slot.get(), const Enemy)) {
                if (hiddenSlot(enemy) == drawHidden) {
                    if (const EnemyComponents::Shell* shell = GetIf(enemy, const EnemyComponents::Shell)) {
                        render(target, *shell, hitboxes);
                    } else if (const Firebar* firebar = GetIf(enemy, const Firebar)) {
                        render(target, *firebar);
                    } else if (const Lift* lift = GetIf(enemy, const Lift)) {
                        render(target, *lift, GetIf(enemy, const LiftBalance));
                    } else if (const Axe* axe = GetIf(enemy, const Axe)) {
                        render(target, *axe);
                    } else if (const NPC* npc = GetIf(enemy, const NPC)) {
                        render(target, *npc);
                    } else {
                        render(target, *enemy, hitboxes);
                    }
                }
            } else if (const JumpSpring* spring = GetIf(slot.get(), const JumpSpring)) {
                render(target, *spring);
            } else if (const Powerup* powerup = GetIf(slot.get(), const Powerup)) {
                if (hiddenSlot(powerup) == drawHidden) {
                    render(target, *powerup, hitboxes);
                }
            } else {
                if (drawHidden) {
                    if (const StarFlag* flag = GetIf(slot.get(), const StarFlag)) {
                        render(target, *flag);
                    }
                } else {
                    if (const Flag* flag = GetIf(slot.get(), const Flag)) {
                        render(target, *flag);
                    }
                }
            }
        }
    }

    if (!drawHidden) {
        if (player.isFiery()) {
            for (const auto& ball : world.m_Fireballs) {
                if (ball) {
                    render(target, *ball);
                }
            }
        }

        for (const auto& sprite : world.m_BouncingCoins) {
            if (sprite) {
                render(target, *sprite);
            }
        }

        for (const auto& sprite : world.m_Fireworks) {
            if (sprite) {
                render(target, *sprite);
            }
        }

        for (const auto& floateyNum : world.m_FloateyNums) {
            if (floateyNum) {
                render(target, floateyNum, 2u, world.CameraPosition);
            }
        }

        for (const auto& animation : world.m_DeathAnimations) {
            if (animation) {
                render(target, *animation);
            }
        }
    }

    target.draw(hitboxes);
}

#pragma region Master

void Renderer::RenderGame(sf::RenderTarget& target, const World& world) noexcept {
    // Note: A sprite is said to be hidden when its drawn behind the foreground tiles

    if (player.IsHidden()) {
        RenderPlayer(target);
    }

    renderSprites(target, world, true); // draw hidden sprites

    renderTiles(target, world);

    renderSprites(target, world, false); // draw non hidden sprites

    if (!player.IsHidden()) {
        RenderPlayer(target);
    }
}

#pragma region Ui

void Renderer::RenderUi(sf::RenderTarget& target, const World& world, const bool& onTitleScreen) noexcept {
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);

    // name
    textAddString(player.Name(), sf::Vector2f(24.f, 10.f), vertices);

    // score
    textAddString(intToStringFixedSize(player.Data.Score, 6u), sf::Vector2f(24.f, 20.f), vertices);

    // coins
    const std::string coinsText = "x" + intToStringFixedSize(player.Data.Coins, 2u);
    textAddString(coinsText, sf::Vector2f(96.f, 20.f), vertices);

    /* coin graphic */ {
        createVertices(sf::Vector2f(88.f, 20.f), sf::Vector2f(0.f, 0.f), sf::Vector2f(8.f, 8.f));

        s_PaletteShader.setUniform("pallete", s_BackgroundPallete);
        s_PaletteShader.setUniform("subPallete", 0.75f);

        sf::RenderStates state(&s_UiTexture);
        state.shader = &s_PaletteShader;

        target.draw(s_Vertices, 6u, sf::PrimitiveType::Triangles, state);
    }

    // world
    textAddString("WORLD", sf::Vector2f(144.f, 10.f), vertices);

    const std::string worldText = std::to_string(player.Data.World) + "-" + std::to_string(player.Data.Level);
    textAddString(worldText, sf::Vector2f(152.f, 20.f), vertices);

    // time
    textAddString("TIME", sf::Vector2f(200.f, 10.f), vertices);

    if (s_RenderGameTime && !onTitleScreen) {
        textAddString(intToStringFixedSize(world.m_GameTime, 3u), sf::Vector2f(204.f, 20.f), vertices);
    }

    textFlush(target, vertices);
}

#pragma region Title Screen

void Renderer::RenderTitleScreen(sf::RenderTarget& target, uint32_t highscore, bool secondPlayerMode) noexcept {
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);

    textAddString("1 PLAYER GAME", sf::Vector2f(88.f, 144.f), vertices);
    textAddString("2 PLAYER GAME", sf::Vector2f(88.f, 160.f), vertices);
    textAddString("TOP- " + intToStringFixedSize(highscore, 6u), sf::Vector2f(96.f, 184.f), vertices);

    textFlush(target, vertices);

    textAddString("c1985 NINTENDO", sf::Vector2f(104.f, 120.f), vertices);

    textFlush(target, vertices, 1u);

    // banner
    createVertices(sf::Vector2f(40.f, 32.f), sf::Vector2f(0.f, 0.f), sf::Vector2f(176.f, 88.f));
    renderVertices(s_BannerTexture, 1u, target);

    // cursor
    createVertices(sf::Vector2f(72.f, 144.f + 16.f * secondPlayerMode), sf::Vector2f(8.f, 0.f), sf::Vector2f(8.f, 8.f));
    renderVertices(s_UiTexture, 1u, target);
}

#pragma region Text

void Renderer::textAddChar(char character, const sf::Vector2f& position, sf::VertexArray& vertices) noexcept {
    if (character >= '0' && character <= '9') { // 0 -> 9
        character -= '0';
    } else if (character >= 'A' && character <= 'Z') { // A -> Z
        character -= 'A' - 10;
    } else if (character == '-') {
        character = 36;
    } else if (character == '.') {
        character = 37;
    } else if (character == 'x') {
        character = 38;
    } else if (character == 'c') {
        character = 40;
    } else if (character == '!') {
        character = 39;
    } else {
        return;
    }

    const float tx_left = character * 8.f;
    const float tx_right = tx_left + 8.f;
    const float tx_top = 0.f;
    const float tx_bottom = 8.f;

    const sf::Vertex topleft = sf::Vertex(position, sf::Color::White, sf::Vector2f(tx_left, tx_top));
    const sf::Vertex topright = sf::Vertex(sf::Vector2f(position.x + 8.f, position.y), sf::Color::White, sf::Vector2f(tx_right, tx_top));
    const sf::Vertex bottomright = sf::Vertex(sf::Vector2f(position.x + 8.f, position.y + 8.f), sf::Color::White, sf::Vector2f(tx_right, tx_bottom));
    const sf::Vertex bottomleft = sf::Vertex(sf::Vector2f(position.x, position.y + 8.f), sf::Color::White, sf::Vector2f(tx_left, tx_bottom));

    vertices.append(topleft);
    vertices.append(std::move(topright));
    vertices.append(bottomright);

    vertices.append(std::move(topleft));
    vertices.append(std::move(bottomright));
    vertices.append(std::move(bottomleft));
}

void Renderer::textAddString(const std::string& string, sf::Vector2f position, sf::VertexArray& vertices) noexcept {
    for (const char& c : string) {
        textAddChar(c, position, vertices);

        position.x += 8.f;
    }
}

void Renderer::textFlush(sf::RenderTarget& target, sf::VertexArray& vertices, uint8_t subPalleteIndex) noexcept {
    s_PaletteShader.setUniform("pallete", s_BackgroundPallete);

    s_PaletteShader.setUniform("subPallete", 0.25f * static_cast<float>(subPalleteIndex));

    sf::RenderStates state(&s_FontTexture);
    state.shader = &s_PaletteShader;

    target.draw(vertices, state);

    vertices.clear();
}