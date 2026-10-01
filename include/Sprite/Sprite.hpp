#pragma once

#include "SFML/System/Vector2.hpp"

#include "Globals.hpp"

class World;

class Sprite {
public:
    Sprite() = default;
    Sprite(sf::Vector2f position) : Position(position) {}
    Sprite(sf::Vector2f position, uint8_t subPaletteIndex) : Position(position), SubPaletteIndex(subPaletteIndex) {}

    virtual ~Sprite() = default;

    [[nodiscard]]
    inline float xPosition() const noexcept {
        return static_cast<float>(static_cast<int>(Position.x));
    }

    [[nodiscard]]
    inline float yPosition() const noexcept {
        return static_cast<float>(static_cast<int>(Position.y));
    }

    virtual void Update(World& world) = 0;

    bool ToRemove{false};

    uint8_t SubPaletteIndex{0u};

    sf::Vector2f Position;
};
