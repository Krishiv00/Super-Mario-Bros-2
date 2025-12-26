#ifndef OTHERS_HPP
#define OTHERS_HPP

#include "Globals.hpp"
#include "SFML/Graphics.hpp"

#include "Blocks.hpp"
#include "Sprite/Sprite.hpp"
#include "Sprite/Powerup.hpp"

class JumpSpring : public Sprite {
private:
    uint8_t m_Timer = 0u;
    uint8_t m_Stage = 0u;

    bool m_BigJump;
    bool m_JumpPressedLastFrame;

    sf::Vector2f m_PivotedPlayerPosition;

public:
    JumpSpring(float position);

    virtual void Update(World& world) override;

    void Activate(World& world);

    [[nodiscard]] inline const uint8_t& getCurrentStage() const {
        return m_Stage;
    }
};

class BouncingBlock : public Sprite {
private:
    std::unique_ptr<Blocks::Block> m_Block;

    const uint8_t& m_BumpTimerRef;

public:
    BouncingBlock() = default;
    BouncingBlock(std::unique_ptr<Blocks::Block>& block, uint8_t item_id, uint8_t& bumpTimerRef, unsigned int x, unsigned int y, uint8_t subPalleteIndex);

    virtual void Update(World& world) override;

    sf::Vector2f getPosition() const;

    uint8_t ItemId;

    [[nodiscard]] inline const Blocks::Block* getBlock() const {
        return m_Block.get();
    }
};

class Flag : public Sprite {
    friend class World;
    friend class Renderer;

private:
    bool m_Moving;

    int8_t m_FloateyNumType = -1;
    uint8_t m_FloateyNumYPos = (gbl::Rows - 4) * TileSize;

public:
    Flag(sf::Vector2f position);

    virtual void Update(World& world) override;

    void SetMoving(bool moving);

    [[nodiscard]] inline bool ReachedBottom() const {
        return Position.y >= 171.f;
    }
};

class StarFlag : public Sprite {
public:
    StarFlag(sf::Vector2f position);

    virtual void Update(World& world) override;
};

class DeathAnimation : public Sprite {
private:
    uint8_t m_Type;
    int8_t m_Direction;

    float m_Velocity;

public:
    DeathAnimation(sf::Vector2f position, uint8_t subPalleteIndex, uint8_t type, int8_t direction, float initialVelocity);

    virtual void Update(World& world) override;

    [[nodiscard]] inline const uint8_t& getType() const {
        return m_Type;
    }
};

class Fireball : public Sprite {
private:
    int8_t m_Direction;
    float m_Velocity;

public:
    Fireball(sf::Vector2f position, bool direction);

    virtual void Update(World& world) override;

    [[nodiscard]] inline sf::FloatRect getHitbox() const {
        return sf::FloatRect(sf::Vector2f(xPosition() + 6.f, yPosition() + 5.f), sf::Vector2f(5.f, 5.f));
    }

    [[nodiscard]] inline const int8_t& getDirection() const {
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

    [[nodiscard]] inline bool Active() const {
        return m_Timer;
    }

    virtual uint8_t GetTextureIndex() const = 0;

    sf::Vector2f Position;
};

class CoinAnimation : public DecorSprite {
public:
    CoinAnimation(sf::Vector2f position);

    virtual void Update() override;

    virtual uint8_t GetTextureIndex() const override;
};

class Firework : public DecorSprite {
private:
    // true ? fireball : confetti
    bool m_Type;

public:
    Firework(sf::Vector2f position, bool type);

    virtual uint8_t GetTextureIndex() const override;
};

class FloateyNum {
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

    [[nodiscard]] inline sf::Vector2f getPosition(float cameraPosition) const {
        return sf::Vector2f(
            static_cast<float>(m_Position.x) + cameraPosition, static_cast<float>(m_Position.y)
        );
    }

    [[nodiscard]] inline const uint8_t& getType() const {
        return m_Type;
    }
};

#endif // !OTHERS_HPP