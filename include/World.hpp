#ifndef WORLD_HPP
#define WORLD_HPP

#include "Globals.hpp"
#include "SFML/Graphics.hpp"

#include "Cutscene.hpp"
#include "Sprite/Enemy.hpp"
#include "Sprite/Others.hpp"
#include "Sprite/Player.hpp"
#include "Blocks.hpp"

class World {
    friend class MapLoader;
    friend class Renderer;
    friend class JumpSpring;
    friend class Cutscene;
    friend class EnemyComponents::Stompable;
    friend class Player;
    friend class LiftBalance;
    friend class BouncingBlock;

public:
    [[nodiscard]] static constexpr inline unsigned int GetIndex(unsigned int x, unsigned int y) noexcept {
        return x * 13u + y;
    }

    static constexpr inline const uint8_t EnemySpriteSlots = 5u;
    static constexpr inline const uint8_t SpecialSpriteSlot = EnemySpriteSlots + 0u;

    static constexpr inline const int8_t MaxSpriteDistanceLeftNormal = 72u;
    static constexpr inline const int8_t MaxSpriteDistanceLeftSpecial = 16u;

    static inline bool Difficulty = false;

private:
    // events
    void on_block_hit_from_bottom(unsigned int x, unsigned int y);
    void on_enter_warp_zone();
    void on_otaining_supermushroom();
    void on_otaining_fireflower();
    void on_otaining_starman();
    void on_player_damage();
    void on_player_death(bool pit_death);

    // routines
    void give_coin();
    void stopCutscene();
    void collectCoinAboveBlock(unsigned int x, unsigned int y);

    // timers
    void updateTimers();

    // multi coin
    bool m_MulticoinTimerActive;
    uint8_t m_MultiCoinTimer;

    // game time
    static constexpr inline const uint8_t GameTimerUpdateLength = 24u;

    void initGameTime(uint16_t duration);
    void updateGameTime();

    uint16_t m_GameTime;
    uint8_t m_GameTimeUpdateTimer;

    // sprites
    void handleSpriteLoading();
    void updateSprites();
    void updateGrowingPowerup();
    void updateFreezeIndependentSprites();
    void activateJumpSpring();
    void spawnCoinAnimation(unsigned int x, unsigned int y);

    std::unique_ptr<Sprite> m_Sprites[EnemySpriteSlots + 1u]; // +1 special slot
    std::vector<std::vector<std::unique_ptr<Sprite>>> m_SpritePool;

    std::unique_ptr<Fireball> m_Fireballs[2u];

    FloateyNum m_FloateyNums[EnemySpriteSlots];
    std::unique_ptr<DecorSprite> m_MiscSprites[2u];
    std::unique_ptr<DeathAnimation> m_DeathAnimations[EnemySpriteSlots];

    // bouncing block
    void handleBlockDefeat(sf::Vector2f blockPosition);
    void updateBouncingBlock();

    uint8_t m_BumpTimer;
    std::unique_ptr<BouncingBlock> m_BouncingBlock;

    // powerup
    void spawn_powerup(uint8_t id, sf::Vector2f position);
    void handlePowerupCollisions();

    // cutscene
    std::unique_ptr<Cutscene> m_Cutscene;

    // collision
    static constexpr inline const uint8_t XCollision = 0b10;
    static constexpr inline const uint8_t YCollision = 0b01;

    static constexpr inline const float TilePenetrationThreshhold = 5.f;

    void collisions_PushOutOfBlockRightwards();
    void collisions_PushOutOfBlockLeftwards();

    bool collisions_CollisionResolveSide(const float& pointX, const float& pointY, const unsigned int& row, const unsigned int& col, std::unique_ptr<Blocks::Block>& block_ptr, bool direction);

    void collisions_LandOnTile(Blocks::Block*& block);
    void collisions_LandOnLift(const float& liftTop);

    void collisions_BonkHead();

    Lift* collisions_PointInLift(const sf::Vector2f& point);

    bool collisions_CoinCheck(std::unique_ptr<Blocks::Block>& block, const unsigned int& col, const unsigned int& row);
    bool collisions_FlagCheck(Blocks::Block* block, const unsigned int& index);
    bool collisions_WarpPipeCheck(Blocks::Block* block);

    void resolvePlayerTileCollisions();

    bool resolvePlayerHeadCollisions(float playerTop);
    bool resolvePlayerFootCollisions(float playerTop);
    bool resolvePlayerSideCollisions(float playerTop);

    uint8_t m_CollisionMode;

    // camera
    void moveCamera();
    void clampPlayerLeft();

    std::vector<std::unique_ptr<Blocks::Block>> m_Tiles;
    std::vector<uint8_t> m_AttributeTable;

    uint8_t m_RequiredCoinsForOneUp = 0;
    bool m_SpawnOneUp;

    bool m_CheckEnemyCollisions;

    bool m_AutoScroll;
    bool m_ScrollLocked;

    bool m_Frozen;

    bool m_NewLevel;
    bool m_NewArea;

    uint8_t m_StompChain;

public:
    void Reset();

    void Update();

    void OnFramerule();

    void TickDownTimer();

    bool PointInTile(const sf::Vector2f& point) const;

    bool AddSprite(std::unique_ptr<Sprite>& sprite);
    void ReplaceSprite(std::unique_ptr<Sprite> sprite, uint8_t slotIndex);

    void SpawnDeathAnimation(sf::Vector2f position, uint8_t subPalleteIndex, uint8_t type, int8_t direction, float initialVelocity, uint8_t slotIndex);

    void SpawnFloateyNum(const FloateyNum& num);
    void SpawnFloateyNum(const FloateyNum& num, uint8_t index);

    bool SpawnFireball(sf::Vector2f position, bool direction);
    void SpawnFirework(sf::Vector2f position, bool type);

    void StartThemeMusic();

    void StartCutscene(std::unique_ptr<Cutscene> scene);

    [[nodiscard]] inline bool cutscenePlaying() const {
        return m_Cutscene != nullptr;
    }

    [[nodiscard]] inline std::unique_ptr<Sprite>* getSprites() {
        return m_Sprites;
    }

    [[nodiscard]] inline const bool& getAutoScroll() const {
        return m_AutoScroll;
    }

    [[nodiscard]] inline const uint8_t& getStompChain() const {
        return m_StompChain;
    }

    [[nodiscard]] inline const uint16_t& getGameTime() const {
        return m_GameTime;
    }

    [[nodiscard]] inline bool reloadRequired() const {
        return m_NewLevel || m_NewArea;
    }

    [[nodiscard]] inline const bool& newLevel() const {
        return m_NewLevel;
    }

    [[nodiscard]] inline const bool& scrollLocked() const {
        return m_ScrollLocked;
    }

    uint8_t CurrentTheme;
    uint8_t HalfwayPage;

    float CameraPosition;

    bool TwoPlayerMode = false;
};

#endif // !WORLD_HPP