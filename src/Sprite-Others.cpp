#include "Sprite/Others.hpp"

#include "Sprite/Player.hpp"
#include "World.hpp"

#pragma region Jump Spring

JumpSpring::JumpSpring(float position) {
    SubPalleteIndex = 2u;
    Position = sf::Vector2f(position, (gbl::Rows - 4) * 16.f);
}

void JumpSpring::Update(World& world) {
    if (!m_Timer) {
        if (Position.x <= (world.CameraPosition - World::MaxSpriteDistanceLeftSpecial)) {
            ToRemove = true;
        }

        return;
    }

    --m_Timer;

    bool movingDown = m_Timer > 5u;

    m_PivotedPlayerPosition.y += movingDown ? 2.f : -2.f;

    // big jump
    if (!player.m_JumpKeyHeld) {
        m_BigJump = false;
    } else if (!m_JumpPressedLastFrame) {
        m_BigJump = m_Timer <= 9u;
    }

    m_JumpPressedLastFrame = player.m_JumpKeyHeld;

    // spring motion
    if (m_Timer == 9u) {
        ++m_Stage;
    } else if (m_Timer == 5u || m_Timer == 1u) {
        --m_Stage;
    } else if (m_Timer == 0u) {
        world.m_CollisionMode = World::XCollision | World::YCollision;
        player.m_State = Player::Walking;
        player.m_AnimationFrame = 2u;
        player.m_Velocity.y = m_BigJump ? -12.f : -7.f;
    }

    player.Position = m_PivotedPlayerPosition;
}

void JumpSpring::Activate(World& world) {
    world.m_CollisionMode = 0u;

    player.m_State = Player::Idle;
    player.m_AnimationFrame = 0u;
    player.m_CurrentGravity = 112u;

    m_Timer = 14u;
    m_Stage = 1u;

    m_BigJump = false;
    m_JumpPressedLastFrame = false;

    m_PivotedPlayerPosition = player.Position;
}

#pragma region Bouncing Block

BouncingBlock::BouncingBlock(std::unique_ptr<Blocks::Block>& block, uint8_t item_id, uint8_t& bumpTimerRef, unsigned int x, unsigned int y, uint8_t subPalleteIndex) : ItemId(item_id), m_BumpTimerRef(bumpTimerRef) {
    Position = sf::Vector2f(static_cast<float>(x), static_cast<float>(y + 2u)) * 16.f;
    m_Block = std::move(block);

    block = std::make_unique<Blocks::Collideable>();

    SubPalleteIndex = subPalleteIndex;
}

void BouncingBlock::Update(World& world) {
    if (m_BumpTimerRef == 0u) {
        sf::Vector2u gridPosition = sf::Vector2u(Position) / 16u;
        world.m_Tiles[World::GetIndex(gridPosition.x, gridPosition.y - 2)] = std::move(m_Block);
    }
}

sf::Vector2f BouncingBlock::getPosition() const {
    constexpr int8_t BounceTable[] = {
        0, -2, 0, 2, 3, 5, 5, 6, 7, 7, 6, 6, 5, 2, 1, 0
    };

    const int8_t offset = BounceTable[m_BumpTimerRef];

    return sf::Vector2f(Position.x, Position.y - static_cast<float>(offset));
}

#pragma region Flag

Flag::Flag(sf::Vector2f position) {
    SubPalleteIndex = 1u;
    m_Moving = false;
    Position = position;
}

void Flag::SetMoving(bool moving) {
    m_Moving = moving;
}

void Flag::Update(World& world) {
    if (m_Moving) {
        Position.y += 2.f;
        m_FloateyNumYPos -= 2;
    }
}

#pragma region Star Flag

StarFlag::StarFlag(sf::Vector2f position) {
    SubPalleteIndex = 2u;
    Position = position;
}

void StarFlag::Update(World&) {

}

#pragma region Death Animation

DeathAnimation::DeathAnimation(sf::Vector2f position, uint8_t subPalleteIndex, uint8_t type, float initialVelocity) : Sprite(position, subPalleteIndex), m_Type(type) {
    m_Velocity = initialVelocity;
}

void DeathAnimation::Update(World& world) {
    Position.y += m_Velocity;
    m_Velocity = std::min(m_Velocity + 0.195f, 3.f);

    ToRemove = Position.y >= gbl::Height;
}

#pragma region Fireball

Fireball::Fireball(sf::Vector2f position, bool direction) : Sprite(position, 2u) {
    m_Direction = direction == gbl::Direction::Right ? 1 : -1;
    m_Velocity = 3.f;
}

void Fireball::Update(World& world) {
    Position.x += 4.f * m_Direction;

    if (m_Direction == 1 ? Position.x >= world.CameraPosition + gbl::Width : Position.x <= std::max(world.CameraPosition - 8.f, 0.f)) {
        ToRemove = true;
        return;
    }

    Position.y += m_Velocity;
    m_Velocity = std::min(m_Velocity + 3.f / 10.f, 3.f);

    float top = yPosition();

    if (top >= gbl::Height) {
        ToRemove = true;
        return;
    }

    if (top >= 215.f || top < 24.f) {
        return;
    }

    float left = xPosition();

    if (m_Velocity > 0.f) {
        sf::Vector2f feetPoint = sf::Vector2f(left + 8.f, top + 8.f);

        if (world.PointInTile(feetPoint)) {
            Position.y = static_cast<int>((top + 16.f) / 16.f) * 16.f - 8.f;
            top = yPosition();
            m_Velocity = -3.f;
        }
    }

    if (top < 56.f) {
        return;
    }

    sf::Vector2f sidePoint = sf::Vector2f(left + 12.f + 4.f * m_Direction, top + 4.f);

    if (world.PointInTile(sidePoint)) {
        ToRemove = true;
        audioPlayer.Play(AudioPlayer::BlockHit);
        world.SpawnFirework(Position, true);
    }
}

#pragma region Coin Animation

CoinAnimation::CoinAnimation(sf::Vector2f position) {
    Position = position;
    m_Timer = 30u;
}

void CoinAnimation::Update() {
    constexpr int8_t Offsets[] = {
        3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0,
        -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, 0
    };

    Position.y += Offsets[m_Timer--];
}

uint8_t CoinAnimation::GetTextureIndex() const {
    return 2 - (m_Timer % 6u) / 2u;
}

#pragma region Firework

Firework::Firework(sf::Vector2f position, bool type) : m_Type(type) {
    Position = position;
    m_Timer = m_Type ? 6u : 23u;
}

uint8_t Firework::GetTextureIndex() const {
    return 6u - m_Timer / (m_Type ? 2u : 8u);
}

#pragma region Floatey Num

FloateyNum::FloateyNum(sf::Vector2f position, float cameraPosition, uint8_t type) : m_Type(type) {
    int16_t xPos = position.x - cameraPosition;

    m_Position = sf::Vector2<uint8_t>(
        static_cast<uint8_t>(std::clamp<int16_t>(xPos, 0, 255 - 16)), static_cast<uint8_t>(position.y)
    );

    m_Timer = 47u;
}

uint8_t FloateyNum::GetType(uint16_t points) {
    constexpr uint8_t Table[] = {1u, 2u, 4u, 5u, 8u, 10u, 20u, 40u, 50u, 80u};

    uint8_t counter = 0u;

    for (; counter < sizeof(Table); ++counter) {
        if (Table[counter] * 100u == points) {
            return counter;
        }
    }

    // one up
    return counter;
}

void FloateyNum::Update() {
    --m_Position.y;
    --m_Timer;
}