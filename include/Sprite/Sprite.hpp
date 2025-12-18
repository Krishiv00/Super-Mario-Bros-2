#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "Globals.hpp"

class World;

class Sprite {
public:
    Sprite() = default;
    Sprite(sf::Vector2f position) : Position(std::move(position)) {}
    Sprite(sf::Vector2f position, uint8_t subPalleteIndex) : Position(std::move(position)), SubPalleteIndex(subPalleteIndex) {}

    virtual ~Sprite() = default;

    [[nodiscard]] inline float xPosition() const noexcept {
        return static_cast<float>(static_cast<int>(Position.x));
    }

    [[nodiscard]] inline float yPosition() const noexcept {
        return static_cast<float>(static_cast<int>(Position.y));
    }

    virtual void Update(World& world) = 0;

    bool ToRemove = false;

    uint8_t SubPalleteIndex = 0u;

    sf::Vector2f Position;
};

#endif // !SPRITE_HPP