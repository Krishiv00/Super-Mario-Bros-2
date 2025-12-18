#ifndef POWERUP_HPP
#define POWERUP_HPP

#include "Globals.hpp"
#include "SFML/Graphics.hpp"

#include "Audio.hpp"

#include "Sprite/Sprite.hpp"

class Powerup : public Sprite {
    friend class Renderer;
    friend class World;
    friend class MapLoader;

    static constexpr inline const float GetOutSpeed = 0.25f;

protected:
    static constexpr inline const uint8_t PalleteAnimationDuration = 2u;

    void handle_movement(World& world);
    virtual void on_feet_collision();

    bool moving_out();

    void animatePallete(uint8_t& animationTimer);

    int8_t m_Direction;

    uint8_t m_GetOutTimer = static_cast<uint8_t>(12.f / GetOutSpeed);

    uint8_t m_Type;

    float m_Velocity = 0.f;

public:
    Powerup() = default;
    Powerup(sf::Vector2f position);
    virtual ~Powerup() = default;

    virtual void Update(World& world) = 0;

    virtual void GrantPower(World& world) = 0;

    [[nodiscard]] inline sf::FloatRect getHitbox() const {
        return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition()), sf::Vector2f(13.f, 13.f));
    }
};

class OneUp : public Powerup {
public:
    OneUp(sf::Vector2f position);

    virtual void Update(World& world) override;

    virtual void GrantPower(World& world) override;
};

class SuperMushroom : public Powerup {
public:
    SuperMushroom(sf::Vector2f position);

    virtual void Update(World& world) override;

    virtual void GrantPower(World& world) override;
};

class FireFlower : public Powerup {
private:

    uint8_t m_PalleteAnimationTimer = PalleteAnimationDuration;

public:
    FireFlower(sf::Vector2f position);

    virtual void Update(World& world) override;

    virtual void GrantPower(World& world) override;
};

class Starman : public Powerup {
private:
    virtual void on_feet_collision() override;

    uint8_t m_PalleteAnimationTimer = PalleteAnimationDuration;

public:
    Starman(sf::Vector2f position);

    virtual void Update(World& world) override;

    virtual void GrantPower(World& world) override;
};

#endif // !POWERUP_HPP