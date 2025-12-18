#ifndef OTHERS_HPP
#define OTHERS_HPP

#include "Globals.hpp"
#include "SFML/Graphics.hpp"

#include "Blocks.hpp"
#include "Sprite/Sprite.hpp"
#include "Sprite/Powerup.hpp"

class JumpSpring : public Sprite {
    friend class Renderer;
    friend class MapLoader;

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
};

class BouncingBlock : public Sprite {
    friend class Renderer;
    friend class MapLoader;

private:
    std::unique_ptr<Blocks::Block> m_Block;

    const uint8_t& m_BumpTimerRef;

public:
    BouncingBlock() = default;
    BouncingBlock(std::unique_ptr<Blocks::Block>& block, uint8_t item_id, uint8_t& bumpTimerRef, unsigned int x, unsigned int y, uint8_t subPalleteIndex);

    virtual void Update(World& world) override;

    sf::Vector2f getPosition() const;

    uint8_t ItemId;
};

class Flag : public Sprite {
    friend class MapLoader;

private:
    static constexpr inline const float Threshold = 171.f;

    bool m_Moving;

public:
    Flag(sf::Vector2f position);

    virtual void Update(World& world) override;

    void SetMoving(bool moving);

    inline bool ReachedBottom() const {
        return Position.y >= Threshold;
    }
};

class StarFlag : public Sprite {
    friend class MapLoader;

public:
    StarFlag(sf::Vector2f position);

    virtual void Update(World& world) override;
};

class DecorSprite {
    friend class MapLoader;

protected:
    uint8_t m_Timer;

public:
    virtual ~DecorSprite() = default;

    virtual void Update() {
        --m_Timer;
    }

    inline bool Active() const {
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
public:
    Firework(sf::Vector2f position);

    virtual uint8_t GetTextureIndex() const override;
};

class FloateyNum {
    friend class Renderer;
    friend class Cutscene;

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

    inline sf::Vector2f getPosition(float cameraPosition) const {
        return sf::Vector2f(
            static_cast<float>(m_Position.x) + cameraPosition, static_cast<float>(m_Position.y)
        );
    }
};

#endif // !OTHERS_HPP