#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Globals.hpp"
#include "SFML/Graphics.hpp"

#include "Sprite/Sprite.hpp"

#include "Audio.hpp"

class World;

class uint24_t {
private:
    uint8_t m_Values[3u]{};

public:
    void operator=(uint32_t val) noexcept;
    void operator+=(uint32_t val) noexcept;

    operator uint32_t() const noexcept;
};

struct PlayerData {
    [[nodiscard]] static constexpr inline uint8_t GetLevelPointer(const uint8_t& world, const uint8_t& level) noexcept {
        return ((world - 1) << 2u) | (level - 1);
    }

    [[nodiscard]] constexpr inline uint8_t GetLevelPointer() const noexcept {
        return GetLevelPointer(World, Level);
    }

    PlayerData(bool type) : Type(type) {}

    bool Type = 0u;
    bool DiedAfterHalfPage = false;

    uint8_t World = 1u;
    uint8_t Level = 1u;
    uint8_t Lives = 3u;
    uint8_t Coins = 0u;

    uint24_t Score;
};

class Player : public Sprite {
    friend class Renderer;
    friend class MapLoader;
    friend class World;
    friend class Cutscene;
    friend class JumpSpring;
    friend class Game;
    friend class ScriptPlayer;
    friend class ScriptRecorder;

public:
    enum Flags : uint8_t {
        Small = 0u,
        Big = 1u,
        Fiery = 2u,

        Idle = 0u,
        Walking = 1u,
        Jumping = 2u,
        Stopping = 3u,
        Climbing = 4u,
        Crouching = 5u,
        Swimming = 6u,
        Changing = 7u,
        Shooting = 8u,
        Dead = 9u,
    };

private:
    // events
    void on_side_collision();
    void on_head_collision();
    void on_feet_collision();

    // input
    static constexpr inline const uint8_t SprintBufferLength = 10u;

    void resetInputs();

    void keyboardControl();
    void joystickControl(uint8_t index);

    void handlePlayerInput();
    void updateInputTimers();
    void on_key_press_left();
    void on_key_press_right();

    uint8_t m_SprintKeyHeld;
    bool m_LeftKeyHeld;
    bool m_RightKeyHeld;
    bool m_UpKeyHeld;
    bool m_DownKeyHeld;
    bool m_JumpKeyHeld;
    bool m_JumpKeyHeldLastFrame;
    bool m_SprintKeyHeldLastFrame;

    bool m_AcceptPlayerControls;
    bool m_TasMode;

    // physics
    static constexpr inline const float MaxWalkingSpeed = 1.5f;
    static constexpr inline const float MaxRunningSpeed = 2.5f;
    static constexpr inline const float MaxWalkingSpeedSwimming = 1.f;

    static constexpr inline const float FrictionNormal = 9.f / 11.f / 16.f;
    static constexpr inline const float FrictionFast = 3.f / 2.f / 16.f;

    float getAcceleration() const;
    float getMaxSpeed() const;
    uint8_t getRisingGravity() const;
    uint8_t getFallingGravity() const;
    void applyFriction();
    void applyGravity();
    void applyMovement();
    void applyVelocity(World& world);
    void clampVelocity();

    sf::Vector2f m_Velocity;

    uint8_t m_CurrentGravity;

    bool m_WasRunningBeforeCurrentAction;

    bool m_SwimmingPhysics;

    // jumping
    void startJump();
    void endJump();
    void handleJump();

    bool m_OnGround;
    bool m_RisingFromJump;

    // fireball
    void handleFireballShooting(World& world);

    // direction
    void updateDirection();

    bool m_Direction;

    // state
    void handleCrouching();
    void updateState();

    uint8_t m_Size;
    uint8_t m_State;
    uint8_t m_StarmanTimer;
    uint8_t m_InvincibilityTimer;

    bool m_Hidden;
    bool m_Visible;
    bool m_Frozen;

    // animation
    uint8_t getAnimationFrames() const;
    uint8_t getAnimationTimer() const;
    void animateFrame();

    uint8_t m_AnimationTimer;
    uint8_t m_AnimationFrame;

    bool m_FireballThrowAnimation;

    // pallete
    static constexpr inline const uint8_t StarmanSlowdownThreshold = 7u;

    void startPalleteAnimation(uint8_t duration);
    void stopPalleteAnimation();
    void animatePallete();

    uint8_t m_PalleteAnimationTimer;
    uint8_t m_PalleteAnimationTimerDuration;

    bool m_AnimatePallete;

    PlayerData m_SecondPlayerData{1u};

public:
    Player();

    void Update(World& world);

    void OnFramerule(World& world);

    void ExtraLife();

    void StartStarman(World& world);

    void Grow(World& world);

    bool Damage(World& world);

    void Kill(World& world, bool pit_death);

    void Reset();

    void Swap();

    void ResetData();

    [[nodiscard]] inline const uint8_t& getState() const noexcept {
        return m_State;
    }

    [[nodiscard]] inline bool isSprinting() const noexcept {
        return m_SprintKeyHeld && (!m_RightKeyHeld || !m_LeftKeyHeld);
    }

    [[nodiscard]] inline bool isCrouching() const noexcept {
        return m_State == Crouching;
    }

    [[nodiscard]] inline bool isFiery() const noexcept {
        return m_Size == Fiery;
    }

    [[nodiscard]] inline bool isBig() const noexcept {
        return m_Size == Big || isFiery();
    }

    [[nodiscard]] inline bool isVisualyBig() const noexcept {
        return isBig() && !isCrouching();
    }

    [[nodiscard]] inline bool HasStarman() const noexcept {
        return m_StarmanTimer;
    }

    [[nodiscard]] inline const bool& IsOnGround() const noexcept {
        return m_OnGround;
    }

    [[nodiscard]] inline const bool& IsHidden() const noexcept {
        return m_Hidden;
    }

    [[nodiscard]] inline const bool& isVisible() const noexcept {
        return m_Visible;
    }

    [[nodiscard]] inline const bool& IsFrozen() const noexcept {
        return m_Frozen;
    }

    [[nodiscard]] inline const bool& IsSwimming() const noexcept {
        return m_SwimmingPhysics;
    }

    [[nodiscard]] inline float currentAbsoluteSpeed() const noexcept {
        return std::fabs(static_cast<float>(static_cast<int>(m_Velocity.x * 16.f)) / 16.f);
    }

    [[nodiscard]] inline bool stillOrWalkingSlowly() const noexcept {
        return currentAbsoluteSpeed() < 1.f;
    }

    [[nodiscard]] inline bool walkingAtFullSpeed() const noexcept {
        float vel = currentAbsoluteSpeed();

        return vel >= 1.f && vel <= MaxWalkingSpeed;
    }

    [[nodiscard]] inline bool runningAtFullSpeed() const noexcept {
        return currentAbsoluteSpeed() > MaxWalkingSpeed;
    }

    [[nodiscard]] inline bool sideButtonsNotPressed() const noexcept {
        return !m_LeftKeyHeld && !m_RightKeyHeld;
    }

    [[nodiscard]] inline const bool& getFacingDirection() const noexcept {
        return m_Direction;
    }

    [[nodiscard]] inline bool facingOppositeDirectionOfMovement() const noexcept {
        return m_Velocity.x && (m_Velocity.x < 0.f ? m_Direction == gbl::Direction::Right : m_Direction == gbl::Direction::Left);
    }

    [[nodiscard]] inline sf::FloatRect getHitbox() const noexcept {
        if (isVisualyBig()) {
            return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 7.f), sf::Vector2f(13.f, 25.f));
        } else {
            return sf::FloatRect(sf::Vector2f(xPosition() + 3.f, yPosition() + 19.f), sf::Vector2f(13.f, 13.f));
        }
    }

    [[nodiscard]] inline const sf::Vector2f& getVelocity() const noexcept {
        return m_Velocity;
    }

    inline void setXVelocity(float vel) noexcept {
        m_Velocity.x = vel;
    }

    inline void setYVelocity(float vel) noexcept {
        m_Velocity.y = vel;
    }

    [[nodiscard]] inline std::string Name() const noexcept {
        return Data.Type ? "LUIGI" : "MARIO";
    }

    PlayerData Data{0u};
};

extern Player player;

#endif // !PLAYER_HPP