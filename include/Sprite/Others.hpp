#pragma once

#include <memory>

#include "Globals.hpp"
#include "SFML/Graphics.hpp"

#include "Blocks.hpp"
#include "Sprite/Sprite.hpp"
#include "Sprite/Powerup.hpp"
#include "Sprite/Enemy.hpp"

class JumpSpring final : public Sprite {
private:
    uint8_t m_Timer{0u};
    uint8_t m_Stage{0u};

    bool m_BigJump;
    bool m_JumpPressedLastFrame;

    sf::Vector2f m_PivotedPlayerPosition;

public:
    JumpSpring(float position);

    virtual void Update(World& world) override;

    void Activate(World& world);

    [[nodiscard]]
    inline uint8_t getCurrentStage() const {
        return m_Stage;
    }
};

class BouncingBlock final : public Sprite {
private:
    std::unique_ptr<Blocks::Block> m_Block;

    const uint8_t& m_BumpTimerRef;

public:
    BouncingBlock(std::unique_ptr<Blocks::Block>& block, uint8_t item_id, uint8_t& bumpTimerRef, unsigned int x, unsigned int y, uint8_t subPaletteIndex);

    virtual void Update(World& world) override;

    sf::Vector2f getPosition() const;

    uint8_t ItemId;

    [[nodiscard]]
    inline const Blocks::Block* getBlock() const {
        return m_Block.get();
    }
};

class Flag final : public Sprite {
    friend class World;
    friend class Renderer;

private:
    bool m_Moving;

    int8_t m_FloateyNumType{-1};
    uint8_t m_FloateyNumYPos{(gbl::Rows - 4) * static_cast<uint8_t>(TileSize)};

public:
    Flag(sf::Vector2f position);

    virtual void Update(World& world) override;

    void SetMoving(bool moving);

    [[nodiscard]]
    inline bool ReachedBottom() const {
        return Position.y >= 171.f;
    }
};

class StarFlag final : public Sprite {
public:
    StarFlag(sf::Vector2f position);

    virtual void Update(World& world) override;
};

class Hammer final : public Sprite {
private:
    static constexpr inline const uint8_t InitialState = 0x10u;

    static constexpr inline const float Gravity = 16.f / 256.f;
    static constexpr inline const float MaxYVelocity = 4.f;

    [[nodiscard]]
    Enemy* getOwner(World& world) const noexcept;

    void cacheOwner(const Enemy& owner) noexcept;
    void followOwner() noexcept;

    void release(World& world);

    void collideWithPlayer(World& world);

    [[nodiscard]]
    bool isOffscreen(float cameraPosition) const noexcept;

    const uint8_t m_OwnerSlot;

    sf::Vector2f m_OwnerPosition;
    int8_t m_OwnerDirection;

    uint8_t m_State;

    float m_XVelocity{0.f};
    float m_YVelocity{0.f};

    bool m_Collided{false};

public:
    Hammer(const Enemy& owner);

    virtual void Update(World& world) override;

    [[nodiscard]]
    inline bool isThrown() const noexcept {
        return m_State == 1u;
    }

    [[nodiscard]]
    sf::FloatRect getHitbox() const;
};

class DeathAnimation final : public Sprite {
private:
    uint8_t m_Type;
    int8_t m_Direction;

    float m_Velocity;

public:
    DeathAnimation(sf::Vector2f position, uint8_t subPaletteIndex, uint8_t type, int8_t direction, float initialVelocity);

    virtual void Update(World& world) override;

    [[nodiscard]]
    inline uint8_t getType() const noexcept {
        return m_Type;
    }
};

class Fireball final : public Sprite {
private:
    int8_t m_Direction;
    float m_Velocity;

public:
    Fireball(sf::Vector2f position, bool direction);

    virtual void Update(World& world) override;

    [[nodiscard]]
    inline sf::FloatRect getHitbox() const {
        return sf::FloatRect(sf::Vector2f(xPosition() + 6.f, yPosition() + 5.f), sf::Vector2f(5.f, 5.f));
    }

    [[nodiscard]]
    inline int8_t getDirection() const noexcept {
        return m_Direction;
    }
};

class DecorSprite {
protected:
    uint8_t m_Timer;

public:
    virtual ~DecorSprite() = default;

    virtual void Update() {
        --m_Timer;
    }

    [[nodiscard]]
    inline bool Active() const noexcept {
        return m_Timer;
    }

    virtual uint8_t GetTextureIndex() const = 0;

    sf::Vector2f Position;
};

class CoinAnimation final : public DecorSprite {
public:
    CoinAnimation(sf::Vector2f position);

    virtual void Update() override;

    virtual uint8_t GetTextureIndex() const override;
};

class Firework final : public DecorSprite {
private:
    // true ? fireball : confetti
    bool m_Type;

public:
    Firework(sf::Vector2f position, bool type);

    virtual uint8_t GetTextureIndex() const override;
};

class FloateyNum final {
private:
    uint8_t m_Timer;
    uint8_t m_Type;

    sf::Vector2<uint8_t> m_Position;

public:
    static uint8_t GetType(uint16_t points);

    FloateyNum() = default;
    FloateyNum(sf::Vector2f position, float cameraPosition, uint8_t type);

    void Update();

    inline void Reset() {
        m_Timer = 0u;
    }

    operator bool() const {
        return m_Timer;
    }

    [[nodiscard]]
    inline sf::Vector2f getPosition(float cameraPosition) const {
        return sf::Vector2f(
            static_cast<float>(m_Position.x) + cameraPosition, static_cast<float>(m_Position.y)
        );
    }

    [[nodiscard]]
    inline uint8_t getType() const noexcept {
        return m_Type;
    }
};
