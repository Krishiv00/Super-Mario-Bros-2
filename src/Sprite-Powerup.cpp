#include "Sprite/Powerup.hpp"
#include "World.hpp"

Powerup::Powerup(sf::Vector2f position) {
    Position = sf::Vector2f(position.x, position.y - 4.f);

    m_Direction = 1;
}

void Powerup::animatePallete(uint8_t& animationTimer) {
    if (--animationTimer == 0u) {
        animationTimer = PalleteAnimationDuration;
        SubPalleteIndex = (SubPalleteIndex + 1u) % 4u;
    }
}

void Powerup::handle_movement(World& world) {
    Position.x += 1.f * m_Direction;

    const float xPos = xPosition();

    const float boundLeft = std::max(world.CameraPosition - World::MaxSpriteDistanceLeftNormal, 0.f);
    const float boundRight = world.CameraPosition + gbl::Width + World::MaxSpriteDistanceLeftNormal;

    if (xPos < boundLeft || (m_Direction == gbl::Direction::Right && xPos > boundRight)) {
        ToRemove = true;
        return;
    }

    Position.y += m_Velocity;

    float top = yPosition();

    if (top >= gbl::Height) {
        ToRemove = true;
        return;
    }

    if (m_Type == gbl::PowerupType::Starman && m_Velocity < 0.f) {
        m_Velocity += (28.f / 256.f);
    } else {
        m_Velocity = std::min(m_Velocity + (61.f / 256.f), 3.f);
    }

    if (top >= 223.f || top < 16.f) {
        return;
    }

    const float left = xPosition();

    if (m_Velocity > 0.f) {
        const sf::Vector2f feetPoint = sf::Vector2f(left + 8.f, top + 16.f);

        if (world.PointInTile(feetPoint)) {
            Position.y = (static_cast<int>((top + 16.f) / 16.f) - 1) * 16.f;
            top = yPosition();

            on_feet_collision();
        }
    }

    if (top < 48.f) {
        return;
    }

    const sf::Vector2f sidePoint = sf::Vector2f(left + 12.f + 4.f * m_Direction, top + 12.f);

    if (world.PointInTile(sidePoint)) {
        m_Direction *= -1;
    }
}

bool Powerup::moving_out() {
    if (m_GetOutTimer) {
        Position.y -= GetOutSpeed;
        --m_GetOutTimer;

        return true;
    }

    return false;
}

void Powerup::on_feet_collision() {
    m_Velocity = 0.f;
}

#pragma region OneUp

OneUp::OneUp(sf::Vector2f position) : Powerup(position) {
    SubPalleteIndex = 1;

    m_Type = gbl::PowerupType::OneUp;
}

void OneUp::GrantPower(World&) {
    player.ExtraLife();
}

void OneUp::Update(World& world) {
    if (!moving_out()) {
        handle_movement(world);
    }
}

#pragma region SuperMushroom

SuperMushroom::SuperMushroom(sf::Vector2f position) : Powerup(position) {
    SubPalleteIndex = 2;

    m_Type = gbl::PowerupType::SuperMushroom;
}

void SuperMushroom::GrantPower(World& world) {
    player.Grow(world);

    audioPlayer.Play(AudioPlayer::PowerupAcquire);
}

void SuperMushroom::Update(World& world) {
    if (!moving_out()) {
        handle_movement(world);
    }
}

#pragma region FireFlower

FireFlower::FireFlower(sf::Vector2f position) : Powerup(position) {
    SubPalleteIndex = 3;

    m_Type = gbl::PowerupType::FireFlower;

    m_Direction = 0;
}

void FireFlower::Update(World&) {
    moving_out();

    animatePallete(m_PalleteAnimationTimer);
}

void FireFlower::GrantPower(World& world) {
    player.Grow(world);

    audioPlayer.Play(AudioPlayer::PowerupAcquire);
}

#pragma region Starman

Starman::Starman(sf::Vector2f position) : Powerup(position) {
    SubPalleteIndex = 1;

    m_Type = gbl::PowerupType::Starman;
    m_Velocity = -1.f;
}

void Starman::on_feet_collision() {
    m_Velocity = -3.f;
}

void Starman::GrantPower(World& world) {
    player.StartStarman(world);

    audioPlayer.Play(AudioPlayer::PowerupAcquire);
}

void Starman::Update(World& world) {
    if (!moving_out()) {
        handle_movement(world);
    }

    animatePallete(m_PalleteAnimationTimer);
}