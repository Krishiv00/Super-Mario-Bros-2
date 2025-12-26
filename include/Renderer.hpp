#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "World.hpp"
#include "Blocks.hpp"
#include "SFML/Graphics.hpp"

class Renderer {
public:
    static inline sf::Color BackgroundColor;

private:
#define RENDER_HITBOXES 0

#if RENDER_HITBOXES
    static void appendHitbox(sf::VertexArray& vertices, const sf::FloatRect& hitbox) noexcept;
#endif // RENDER_HITBOXES

    static void createVertices(const sf::Vector2f& position, const sf::Vector2f& texturePosition, const sf::Vector2f& size, bool flipHorizontally, bool flipVertically) noexcept;
    static void renderVertices(const sf::Texture& texture, const uint8_t& subPalleteIndex, sf::RenderTarget& target) noexcept;

    static void render(sf::RenderTarget& target, const uint8_t& textureId, const uint8_t& subPalleteIndex, sf::Vector2f position) noexcept;
    static void render(sf::RenderTarget& target, const Enemy& enemy, sf::VertexArray& hitboxes) noexcept;
    static void render(sf::RenderTarget& target, const NPC& npc) noexcept;
    static void render(sf::RenderTarget& target, const EnemyComponents::Shell& shell, sf::VertexArray& hitboxes) noexcept;
    static void render(sf::RenderTarget& target, const Firebar& firebar) noexcept;
    static void render(sf::RenderTarget& target, const Powerup& powerup, sf::VertexArray& hitboxes) noexcept;
    static void render(sf::RenderTarget& target, const JumpSpring& spring) noexcept;
    static void render(sf::RenderTarget& target, const Flag& flag) noexcept;
    static void render(sf::RenderTarget& target, const StarFlag& flag) noexcept;
    static void render(sf::RenderTarget& target, const Axe& axe) noexcept;
    static void render(sf::RenderTarget& target, const Lift& lift, bool balanceLift) noexcept;
    static void render(sf::RenderTarget& target, const Fireball& ball) noexcept;
    static void render(sf::RenderTarget& target, const DecorSprite& sprite) noexcept;
    static void render(sf::RenderTarget& target, const FloateyNum& floateyNum, uint8_t subPalleteIndex, float cameraPos) noexcept;
    static void render(sf::RenderTarget& target, const DeathAnimation& animation) noexcept;

    static void renderTiles(sf::RenderTarget& target, const World& world) noexcept;
    static void renderSprites(sf::RenderTarget& target, const World& world, bool drawHidden) noexcept;

    static bool hiddenSlot(const Enemy* enemy) noexcept;
    static bool hiddenSlot(const Powerup* powerup) noexcept;

    static std::string intToStringFixedSize(unsigned int _int, uint8_t length) noexcept;
    static sf::Vector2f fireballTexturePos();

    static void textAddChar(char character, const sf::Vector2f& position, sf::VertexArray& vertices) noexcept;
    static void textAddString(const std::string& string, sf::Vector2f position, sf::VertexArray& vertices) noexcept;
    static void textFlush(sf::RenderTarget& target, sf::VertexArray& vertices, uint8_t subPalleteIndex = 2u) noexcept;

    static inline sf::Texture s_PlayerTexture;
    static inline sf::Texture s_SpritesTexture;
    static inline sf::Texture s_PowerupsTexture;
    static inline sf::Texture s_TilesetTexture;
    static inline sf::Texture s_UiTexture;
    static inline sf::Texture s_JumpSpringTexture;
    static inline sf::Texture s_BannerTexture;
    static inline sf::Texture s_FloateyNumsTexture;
    static inline sf::Texture s_MiscSpritesTexture;
    static inline sf::Texture s_FontTexture;

    static inline sf::Texture s_BackgroundPallete;
    static inline sf::Texture s_SpritePallete;

    static inline sf::Shader s_PaletteShader;

    static inline sf::Vertex s_Vertices[6u];

    static constexpr inline const uint8_t AnimationTimerDuration = 8u;
    static inline uint8_t s_BlinkAnimationTimer = AnimationTimerDuration;
    static inline uint8_t s_SpriteAnimationTimer = AnimationTimerDuration;

    static inline char s_BlinkDirection = 1;

    static inline uint8_t s_BlinkAnimation = 0u;
    static inline bool s_EnemyAnimation = 0u;

    static inline bool s_RenderGameTime = true;

public:
    static void LoadResources() noexcept;

    static void SetBackgroundTheme(const bool& skyColIndex, const uint8_t& folliageColsIndex, const uint8_t& bodyColsIndex) noexcept;
    static void SetSpriteTheme(const uint8_t& theme, uint8_t endIndex = 12u) noexcept;
    static void SetPlayerTheme(const uint8_t& theme) noexcept;
    static void SetGameTimeRendering(bool state) noexcept;

    static void Animate() noexcept;
    static void ResetAnimations() noexcept;
    static void UpdatePalleteColors() noexcept;

    static void RenderUi(sf::RenderTarget& target, const World& world, const bool& onTitleScreen) noexcept;
    static void RenderGame(sf::RenderTarget& target, const World& world) noexcept;
    static void RenderPlayer(sf::RenderTarget& target) noexcept;

    static void RenderBlackScreen_LevelTransition(sf::RenderTarget& target) noexcept;
    static void RenderBlackScreen_TimeUp(sf::RenderTarget& target, bool twoPlayerMode) noexcept;
    static void RenderBlackScreen_GameOver(sf::RenderTarget& target, bool twoPlayerMode) noexcept;

    static void RenderTitleScreen(sf::RenderTarget& target, uint32_t highscore, bool secondPlayerMode) noexcept;

    [[nodiscard]] static inline const bool& getEnemyAnimation() noexcept {
        return s_EnemyAnimation;
    }
};

#endif // !RENDERER_HPP