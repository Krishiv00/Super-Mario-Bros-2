#include "Sprite/Enemy.hpp"
#include "World.hpp"

#include "Renderer.hpp"

#pragma region Enemy

Enemy::Enemy(EnemyType::Type type, sf::Vector2f position) : m_Type(type) {
    Position = position;
}

uint16_t Enemy::GetStompScore(const uint8_t& stompChain) noexcept {
    constexpr uint8_t StompChainPoints[] = {1u, 2u, 4u, 5u, 8u, 10u, 20u, 40u, 50u, 80u};

    if (stompChain < 10u) {
        return StompChainPoints[stompChain] * 100u;
    } else {
        return 0u; // to be interpreted as a one up
    }
}

uint16_t Enemy::GetShellScore(const uint8_t& shellChain) noexcept {
    constexpr uint8_t ShellChainPoints[] = {5u, 8u, 10u, 20u, 40u, 50u, 80u};

    if (shellChain < 7u) {
        return ShellChainPoints[shellChain] * 100u;
    } else {
        return 0u; // to be interpreted as a one up
    }
}

void Enemy::spawnDeathAnimation(World& world) noexcept {
    float offset = (m_Type != EnemyType::HammerBrother) * TileSize;
    world.m_DeathAnimations[SlotIndex] = std::make_unique<DeathAnimation>(sf::Vector2f(Position.x, Position.y + offset), SubPalleteIndex, m_Type);
}

void Enemy::givePlayerScore(uint16_t score, World& world) noexcept {
    player.Data.Score += score;
    world.SpawnFloateyNum(FloateyNum(sf::Vector2f(xPosition(), yPosition()), world.CameraPosition, FloateyNum::GetType(score)), SlotIndex);
}

void Enemy::givePlayerLife(World& world) noexcept {
    player.ExtraLife();
    world.SpawnFloateyNum(FloateyNum(sf::Vector2f(xPosition(), yPosition()), world.CameraPosition, FloateyNum::GetType(0u)), SlotIndex);
}

void Enemy::setDirectionRelativeToPlayer() noexcept {
    m_Direction = -gbl::sign(xPosition() - player.xPosition());
}

void Enemy::onShellDeath(World& world, uint8_t killChain) noexcept {
    ToRemove = true;
    audioPlayer.Play(AudioPlayer::Kick);

    spawnDeathAnimation(world);

    if (uint16_t score = GetShellScore(killChain)) {
        givePlayerScore(score, world);
    } else {
        givePlayerLife(world);
    }
}

bool Enemy::shouldDespawn(float cameraPosition, float maxThreshold = World::MaxSpriteDistanceLeftNormal) const noexcept {
    float boundLeft = std::max(cameraPosition - maxThreshold, 0.f);
    float boundRight = cameraPosition + gbl::Width + World::MaxSpriteDistanceLeftNormal;

    // height of the game (240 pixels)
    constexpr float boundBottom = gbl::Height;

    float enemyX = xPosition();
    float enemyY = yPosition();

    return enemyX < boundLeft || (m_Direction == gbl::Direction::Right && enemyX > boundRight) || enemyY > boundBottom;
}

void Enemy::onCollide(World& world) {
    if (player.Damage(world) && m_Type != EnemyType::PiranhaPlant && m_Type != EnemyType::Firebar && !GetIf(this, EnemyComponents::Shell)) {
        setDirectionRelativeToPlayer();
    }
}

void Enemy::onBlockDefeat(World& world, float) {
    ToRemove = true;
    spawnDeathAnimation(world);
}

void Enemy::OnCollisionWithPlayer(World& world) {
    if (m_TouchingPlayer) {
        return;
    }

    m_TouchingPlayer = true;

    if (player.HasStarman()) {
        ToRemove = true;
        audioPlayer.Play(AudioPlayer::StarmanKill);

        uint16_t score;

        if (m_Type == EnemyType::Goomba) {
            score = 100u;
        } else if (m_Type == EnemyType::HammerBrother) {
            score = 1000u;
        } else {
            score = 200u;
        }

        givePlayerScore(score, world);

        spawnDeathAnimation(world);
    } else {
        EnemyComponents::Stompable* stompable = GetComponent(this, EnemyComponents::Stompable);

        if (!stompable || !stompable->isStomped()) {
            onCollide(world);
        } else {
            ToRemove = true;

            audioPlayer.Play(AudioPlayer::Stomp);

            stompable->onStomp(world);
        }
    }
}

void Enemy::OnNoCollision() noexcept {
    m_TouchingPlayer = false;
}

#pragma region Enemy Components

namespace EnemyComponents {
    void SideToSideMovement::update(World& world, float speed = 0.5f, bool make_sound = false) {
        Position.x += speed * m_Direction;

        float top = yPosition();

        if (top >= 32.f && top < 207.f) {
            float bottom = top + TileSize * 2.f;

            bool movingRight = m_Direction == 1u;

            sf::Vector2f sidePoint = sf::Vector2f(xPosition() + TileSize * movingRight, bottom - 1.f);

            if (world.PointInTile(sidePoint)) {
                m_Direction *= -1;

                if (make_sound) {
                    audioPlayer.Play(AudioPlayer::BlockHit);
                }
            }
        }
    }

    void GravityMovement::update(World& world, float gravityForce = 0.195f) {
        m_YVelocity = std::min(m_YVelocity + gravityForce, 3.f);

        Position.y += m_YVelocity;

        float top = yPosition();

        if (top >= 16.f && top < 207.f) {
            float bottom = top + TileSize * 2.f;

            sf::Vector2f feetPoint = sf::Vector2f(xPosition() + TileSize * 0.5f, bottom);

            if (m_OnGround = world.PointInTile(feetPoint)) {
                Position.y = (static_cast<int>(bottom / TileSize) - 2) * TileSize;
                m_YVelocity = 0.f;
            }
        }
    }

    void CollideWithOtherEnemies::update(World& world) {
        auto sprites = world.getSprites();

        for (uint8_t i = SlotIndex + 1u; i < World::EnemySpriteSlots; ++i) {
            auto& slot = sprites[i];
            if (!slot) continue;

            Enemy* enemy = GetComponent(slot.get(), Enemy);
            if (!enemy) continue;

            if (
                CollideWithOtherEnemies* other = GetComponent(enemy, CollideWithOtherEnemies);
                other &&
                other->m_Direction != m_Direction &&
                other->getHitbox().findIntersection(getHitbox())
            ) {
                m_Direction *= -1;

                if (!GetIf(other, Shell)) {
                    other->m_Direction *= -1;
                }
            }
        }
    }

    bool Stompable::isStomped() {
        return !player.IsSwimming() && (
            player.getVelocity().y > 0.f ||
            player.getVelocity().y == -4.f ||
            player.yPosition() + 12.f < yPosition()
        );
    }

    void Stompable::onStomp(World& world) {
        float playerLastVel = player.getVelocity().y;

        player.setYVelocity(-4.f);

        uint16_t score;

        if (m_Type == EnemyType::KoopaParatroopa) {
            score = 400u;
        } else if (m_Type == EnemyType::HammerBrother) {
            score = 1000u;
        } else if (m_Type == EnemyType::CheepCheep) {
            score = 200u;
        } else if (m_Type == EnemyType::Lakitu) {
            score = 800u;
        } else if (m_Type == EnemyType::BulletBill) {
            score = 200u;
        } else {
            score = GetStompScore(world.m_StompChain++ + (playerLastVel == -4.f));
        }

        if (score) {
            givePlayerScore(score, world);
        } else {
            givePlayerLife(world);
        }
    }

    void GroundEnemy::HandleMovement(World& world) {
        SideToSideMovement::update(world, World::Difficulty ? 0.75f : 0.5f);
        GravityMovement::update(world);
    }

    void GroundEnemy::Update(World& world) {
        if (shouldDespawn(world.CameraPosition)) {
            ToRemove = true;
            return;
        }

        CollideWithOtherEnemies::update(world);
    }

    std::unique_ptr<Shell> ShellEnemy::getShellObj() const {
        std::unique_ptr<Shell> shell;

        if (m_Type == EnemyType::KoopaTroopa) {
            shell = std::make_unique<KoopaTroopaShell>(Position);
        } else if (m_Type == EnemyType::RedKoopaTroopa) {
            shell = std::make_unique<RedKoopaShell>(Position);
        } else if (m_Type == EnemyType::BuzzyBeetle) {
            shell = std::make_unique<BuzzyBeetleShell>(Position);
        } else {
            return nullptr;
        }

        shell->SubPalleteIndex = SubPalleteIndex;

        return shell;
    }

    void ShellEnemy::onBlockDefeat(World& world, float blockPosition) {
        Enemy::onBlockDefeat(world, blockPosition);

        std::unique_ptr<Shell> shell = getShellObj();
        shell->m_FlippedVertically = true;
        shell->m_YVelocity = -3.f;
        shell->m_Direction = xPosition() > blockPosition ? 1 : -1;
        shell->m_Animate = true;

        world.ReplaceSprite(std::move(shell), SlotIndex);
    }

    void ShellEnemy::onStomp(World& world) {
        Stompable::onStomp(world);

        std::unique_ptr<Shell> shell = getShellObj();

        if (!m_OnGround) {
            shell->m_FlippedVertically = true;
        }

        world.ReplaceSprite(std::move(shell), SlotIndex);
    }

    Shell::Shell() {
        m_KillChain = 0u;
        m_FlippedVertically = false;

        setRestState();
    }

    void Shell::HandleMovement(World& world) {
        if (m_Moving) {
            SideToSideMovement::update(world, 1.5f * (m_OnGround + 1u), true);
        } else if (!m_OnGround && m_FlippedVertically) {
            SideToSideMovement::update(world, 1.f);
        }

        bool oldGround = m_OnGround;

        GravityMovement::update(world);

        // flipped and just landed
        if (m_FlippedVertically && !m_Moving && m_OnGround && !oldGround) {
            m_Animate = false;
            m_Direction = 0;
        }
    }

    void Shell::Update(World& world) {
        if (shouldDespawn(world.CameraPosition)) {
            ToRemove = true;
            return;
        }

        if (m_Moving) {
            killOtherEnemies(world);
        } else {
            CollideWithOtherEnemies::update(world);
        }
    }

    void Shell::killOtherEnemies(World& world) {
        auto sprites = world.getSprites();

        for (uint8_t i = 0u; i < World::EnemySpriteSlots; ++i) {
            if (i == SlotIndex) continue;

            auto& slot = sprites[i];
            if (!slot) continue;

            if (
                Enemy* other = GetComponent(slot.get(), Enemy);
                other &&
                !GetIf(other, DeadGoomba) &&
                !GetIf(other, Lift) &&
                other->getHitbox().findIntersection(getHitbox())
            ) {
                other->onShellDeath(world, m_KillChain++);
            }
        }
    }

    void Shell::setMovingState() {
        m_Direction = gbl::sign(xPosition() - player.xPosition());

        m_Moving = true;

        m_Animate = false;

        audioPlayer.Play(AudioPlayer::Kick);
    }

    void Shell::setRestState() {
        m_Direction = 0;

        m_Moving = false;

        m_RevivalTimer = World::Difficulty ? 11u : 16u;

        m_Animate = false;
    }

    void Shell::giveKickPoints(World& world) {
        uint16_t score;

        if (!m_OnGround) {
            score = 8000u;
        } else if (m_Animate) {
            score = Renderer::getEnemyAnimation() ? 8000u : 400u;
        } else if (world.getStompChain() > 0u) {
            score = 500u;
        } else {
            score = 400u;
        }

        givePlayerScore(score, world);
    }

    void Shell::onStomp(World& world) {
        if (m_Moving) {
            Stompable::onStomp(world);
            setRestState();
        } else {
            giveKickPoints(world);
            setMovingState();
        }

        m_FlippedVertically = false;

        // stop shell from being killed from a stomp
        ToRemove = false;
    }

    void Shell::onCollide(World& world) {
        if (m_Moving) {
            player.Damage(world);
        } else {
            giveKickPoints(world);

            setMovingState();
        }
    }

    void Shell::OnFramerule(World& world) {
        if (m_Moving) {
            return;
        }

        --m_RevivalTimer;

        if (m_RevivalTimer != 0u) {
            if (m_RevivalTimer == 4u) {
                m_Animate = true;
            }

            return;
        }

        std::unique_ptr<ShellEnemy> enemy;

        if (m_Type == EnemyType::KoopaTroopaShell) {
            enemy = std::make_unique<KoopaTroopa>(Position);
        } else if (m_Type == EnemyType::RedKoopaShell) {
            enemy = std::make_unique<RedKoopaTroopa>(Position);
        } else if (m_Type == EnemyType::BuzzyBeetleShell) {
            enemy = std::make_unique<BuzzyBeetle>(Position);
        } else return;

        // 50-50 chance that enemy moves to right when woken up
        if (Rand::RandomInt(Rand::OffsetSpawning) & 0x01u) {
            enemy->m_Direction *= -1;
        }

        world.ReplaceSprite(std::move(enemy), SlotIndex);
    }

    float OscillatingMovement::getOffset(uint8_t phaseDuration, uint8_t halfwayDistance) {
        constexpr auto EaseIn = [](float t) -> float {
            return t * t * t;
        };

        constexpr auto EaseOut = [](float t) -> float {
            t = 1.f - t;
            return 1.f - t * t * t;
        };

        constexpr uint8_t PhaseBase[] = {0u, 1u, 2u, 1u};
        constexpr int8_t PhaseDir[] = {1, 1, -1, -1};

        const float normalizedFrame = (m_Phase % 2u ? EaseOut : EaseIn)(static_cast<float>(m_Frame) / static_cast<float>(phaseDuration));
        const float distanceScalar = static_cast<float>(PhaseBase[m_Phase]) + static_cast<float>(PhaseDir[m_Phase]) * normalizedFrame;

        if (++m_Frame > phaseDuration) {
            m_Frame = 0u;
            m_Phase = (m_Phase + 1u) % 4u;
        }

        return static_cast<float>(halfwayDistance) * distanceScalar;
    }
}

#pragma region Goomba

Goomba::Goomba(sf::Vector2f position) : Enemy(EnemyType::Goomba, position) {
    SubPalleteIndex = 3u;
}

void Goomba::onStomp(World& world) {
    Stompable::onStomp(world);

    world.ReplaceSprite(std::make_unique<DeadGoomba>(Position), SlotIndex);
}

sf::FloatRect Goomba::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 3.f, yPosition() + 21.f), sf::Vector2f(11.f, 7.f));
}

#pragma region Dead Goomba

DeadGoomba::DeadGoomba(sf::Vector2f position) : Enemy(EnemyType::DeadGoomba, position) {
    SubPalleteIndex = 3u;

    m_Direction = 0;

    m_Animate = false;

    m_Timer = 36u;
}

void DeadGoomba::HandleMovement(World& world) {
    GravityMovement::update(world);
    CollideWithOtherEnemies::update(world);
}

void DeadGoomba::Update(World&) {
    if (--m_Timer == 0u) {
        ToRemove = true;
    }
}

sf::FloatRect DeadGoomba::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 3.f, yPosition() + 21.f), sf::Vector2f(11.f, 7.f));
}

#pragma region Koopa Troopa

KoopaTroopa::KoopaTroopa(sf::Vector2f position) : Enemy(EnemyType::KoopaTroopa, position) {
    SubPalleteIndex = 1u;
}

sf::FloatRect KoopaTroopa::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 16.f), sf::Vector2f(12.f, 13.f));
}

#pragma region Koopa Troopa Shell

KoopaTroopaShell::KoopaTroopaShell(sf::Vector2f position) : Enemy(EnemyType::KoopaTroopaShell, position) {

}

sf::FloatRect KoopaTroopaShell::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 16.f), sf::Vector2f(12.f, 13.f));
}

#pragma region Koopa Paratroopa

KoopaParatroopa::KoopaParatroopa(sf::Vector2f position) : Enemy(EnemyType::KoopaParatroopa, position) {
    SubPalleteIndex = 1u;
}

void KoopaParatroopa::HandleMovement(World& world) {
    SideToSideMovement::update(world);
    GravityMovement::update(world, 1.f / 9.5f);
}

void KoopaParatroopa::Update(World& world) {
    if (shouldDespawn(world.CameraPosition)) {
        ToRemove = true;
        return;
    }

    CollideWithOtherEnemies::update(world);

    if (m_OnGround) {
        m_YVelocity = -3.f;
    }
}

void KoopaParatroopa::onStomp(World& world) {
    Stompable::onStomp(world);

    std::unique_ptr<KoopaTroopa> koopa = std::make_unique<KoopaTroopa>(Position);

    koopa->setDirectionRelativeToPlayer();

    world.ReplaceSprite(std::move(koopa), SlotIndex);
}

sf::FloatRect KoopaParatroopa::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 16.f), sf::Vector2f(13.f, 13.f));
}

#pragma region Red Koopa Troopa

RedKoopaTroopa::RedKoopaTroopa(sf::Vector2f position) : Enemy(EnemyType::RedKoopaTroopa, position) {
    SubPalleteIndex = 2u;
}

void RedKoopaTroopa::Update(World& world) {
    KoopaTroopa::Update(world);

    // turn around
    if (!m_OnGround) {
        m_Direction *= -1;
    }
}

#pragma region Red Koopa paratroopa

RedKoopaParatroopa::RedKoopaParatroopa(sf::Vector2f position) : Enemy(EnemyType::RedKoopaParatroopa, position) {
    SubPalleteIndex = 2u;

    m_Starty = position.y;
}

void RedKoopaParatroopa::HandleMovement(World&) {
    Position.y = m_Starty + OscillatingMovement::getOffset(100u, 52u);
}

void RedKoopaParatroopa::Update(World& world) {
    if (shouldDespawn(world.CameraPosition)) {
        ToRemove = true;
        return;
    }
}

void RedKoopaParatroopa::onStomp(World& world) {
    Stompable::onStomp(world);

    std::unique_ptr<KoopaTroopa> koopa = std::make_unique<KoopaTroopa>(Position);

    // spawn normal koopa just with red pallete
    koopa->SubPalleteIndex = SubPalleteIndex;
    koopa->setDirectionRelativeToPlayer();

    world.ReplaceSprite(std::move(koopa), SlotIndex);
}

sf::FloatRect RedKoopaParatroopa::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 16.f), sf::Vector2f(13.f, 13.f));
}

#pragma region Piranha Plant

PiranhaPlant::PiranhaPlant(sf::Vector2f position) : Enemy(EnemyType::PiranhaPlant, position) {
    SubPalleteIndex = 1u;

    m_PauseTimer = 64u;
}

void PiranhaPlant::HandleMovement(World&) {
    constexpr float Speed = 0.5f;

    if (m_PauseTimer) {
        if ((m_Direction == -1 || std::fabs(player.xPosition() - xPosition()) > 32.f) && --m_PauseTimer == 0u) {
            m_MovementTimer = static_cast<uint8_t>(24.f / Speed);
            m_Direction *= -1;
        }
    } else {
        Position.y += Speed * m_Direction;

        if (--m_MovementTimer == 0u) {
            m_PauseTimer = 64u;
        }
    }
}

void PiranhaPlant::Update(World& world) {
    if (shouldDespawn(world.CameraPosition, World::MaxSpriteDistanceLeftSpecial)) {
        ToRemove = true;
        return;
    }
}

sf::FloatRect PiranhaPlant::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 3.f, yPosition() + 21.f), sf::Vector2f(11.f, 7.f));
}

#pragma region Buzzy Beetle

BuzzyBeetle::BuzzyBeetle(sf::Vector2f position) : Enemy(EnemyType::BuzzyBeetle, position) {
    SubPalleteIndex = 3u;
}

sf::FloatRect BuzzyBeetle::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 16.f), sf::Vector2f(12.f, 13.f));
}

#pragma region Buzzy Beetle Shell

BuzzyBeetleShell::BuzzyBeetleShell(sf::Vector2f position) : Enemy(EnemyType::BuzzyBeetleShell, position) {
    m_Animate = false;
}

sf::FloatRect BuzzyBeetleShell::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 16.f), sf::Vector2f(12.f, 13.f));
}

#pragma region Spiny Egg

SpinyEgg::SpinyEgg(sf::Vector2f position) : Enemy(EnemyType::SpinyEgg, position) {
    SubPalleteIndex = 2u;
}

void SpinyEgg::HandleMovement(World& world) {
    GravityMovement::update(world);
}

void SpinyEgg::Update(World& world) {
    if (shouldDespawn(world.CameraPosition)) {
        ToRemove = true;
        return;
    }

    if (m_YVelocity == 0.f) {
        std::unique_ptr<Spiny> spiny = std::make_unique<Spiny>(Position);
        spiny->setDirectionRelativeToPlayer();

        world.ReplaceSprite(std::move(spiny), SlotIndex);
    }
}

sf::FloatRect SpinyEgg::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 22.f), sf::Vector2f(13.f, 5.f));
}

#pragma region Spiny

Spiny::Spiny(sf::Vector2f position) : Enemy(EnemyType::Spiny, position) {
    SubPalleteIndex = 2u;
}

sf::FloatRect Spiny::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 21.f), sf::Vector2f(11.f, 7.f));
}

#pragma region Bloober

Bloober::Bloober(sf::Vector2f position) : Enemy(EnemyType::Bloober, position) {
    SubPalleteIndex = 3u;

    m_Animate = false;
}

void Bloober::HandleMovement(World&) {

}

void Bloober::Update(World& world) {
    if (shouldDespawn(world.CameraPosition)) {
        ToRemove = true;
        return;
    }
}

sf::FloatRect Bloober::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 3.f, yPosition() + 18.f), sf::Vector2f(11.f, 7.f));
}

#pragma region Cheep Cheep

CheepCheep::CheepCheep(sf::Vector2f position) : Enemy(EnemyType::CheepCheep, position) {
    SubPalleteIndex = 1u;
}

void CheepCheep::HandleMovement(World&) {
    Position.x -= World::Difficulty ? 0.75f : 0.5f;
}

void CheepCheep::Update(World& world) {
    if (shouldDespawn(world.CameraPosition)) {
        ToRemove = true;
        return;
    }
}

sf::FloatRect CheepCheep::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 3.f, yPosition() + 21.f), sf::Vector2f(11.f, 7.f));
}

#pragma region Lakitu

Lakitu::Lakitu(sf::Vector2f position) : Enemy(EnemyType::Lakitu, position) {
    SubPalleteIndex = 1u;

    m_Animate = false;
}

void Lakitu::HandleMovement(World&) {

}

void Lakitu::Update(World& world) {
    if (shouldDespawn(world.CameraPosition)) {
        ToRemove = true;
        return;
    }
}

sf::FloatRect Lakitu::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 16.f), sf::Vector2f(13.f, 13.f));
}

#pragma region Hammer Brother

HammerBrother::HammerBrother(sf::Vector2f position) : Enemy(EnemyType::HammerBrother, position) {
    SubPalleteIndex = 1u;
}

void HammerBrother::HandleMovement(World& world) {
    GravityMovement::update(world);

    m_Direction = gbl::sign(player.xPosition() - xPosition());
}

void HammerBrother::Update(World& world) {
    if (shouldDespawn(world.CameraPosition, World::MaxSpriteDistanceLeftSpecial)) {
        ToRemove = true;
        return;
    }
}

sf::FloatRect HammerBrother::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 2.f, yPosition() + 11.f), sf::Vector2f(9.f, 25.f));
}

#pragma region Bullet Bill

BulletBill::BulletBill(sf::Vector2f position) : Enemy(EnemyType::BulletBill, position) {
    SubPalleteIndex = 1u;
}

void BulletBill::HandleMovement(World&) {
    Position.x -= World::Difficulty ? 0.75f : 0.5f;
}

void BulletBill::Update(World& world) {
    if (shouldDespawn(world.CameraPosition)) {
        ToRemove = true;
        return;
    }
}

sf::FloatRect BulletBill::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 3.f, yPosition() + 21.f), sf::Vector2f(11.f, 7.f));
}

#pragma region Firebar

Firebar::Firebar(sf::Vector2f position, bool size, bool direction, bool fast) : Enemy(EnemyType::Firebar, position), m_Size(6u * (size + 1u)), m_Speed(fast) {
    SubPalleteIndex = 2u;

    m_MoveState = false;
    m_MoveTimer = 6u;
    m_Angle = Rand::RandomInt(Rand::OffsetSpawning) % 32u;

    m_Direction = direction;

    Position = position;
}

void Firebar::HandleMovement(World&) {
    if (--m_MoveTimer == 0u) {
        constexpr uint8_t Timers[] = {6u, 7u};

        m_MoveState ^= true;
        m_MoveTimer = m_Speed ? (Timers[m_MoveState] - 2) : Timers[m_MoveState];

        if (m_Direction == gbl::Direction::Left) {
            ++m_Angle;
        } else {
            --m_Angle;
        }

        m_Angle %= 32u;
    }
}

void Firebar::Update(World& world) {
    if (shouldDespawn(world.CameraPosition)) {
        ToRemove = true;
        return;
    }

    handleCollision(world);
}

void Firebar::handleCollision(World& world) {
    constexpr float ballRadius = TileSize * 0.5f;

    const float cx = Position.x + TileSize * 0.5f;
    const float cy = Position.y + TileSize * 0.5f;

    const float px = player.xPosition() + 8.f;
    const float py = player.yPosition() + (player.isVisualyBig() ? 16.f : 24.f);

    const float angleRad = getAngle();
    const float angleSin = std::sin(angleRad);
    const float angleCos = std::cos(angleRad);

    uint8_t ballChecks = m_Size - 1;

    for (uint8_t i = 0u; i < ballChecks; ++i) {
        float radius = i * TileSize * 0.5f;

        const float fx = cx + angleSin * radius;
        const float fy = cy + angleCos * radius;

        const float dx = px - fx;
        const float dy = py - fy;

        const float distSq = dx * dx + dy * dy;
        const float radiusSq = ballRadius * ballRadius;

        if (distSq <= radiusSq) {
            return OnCollisionWithPlayer(world);
        }
    }

    return OnNoCollision();
}

#pragma region Podoboo

Podoboo::Podoboo(sf::Vector2f position) : Enemy(EnemyType::Podoboo, position) {
    SubPalleteIndex = 2u;

    m_Animate = false;
}

void Podoboo::HandleMovement(World&) {

}

void Podoboo::Update(World& world) {
    if (shouldDespawn(world.CameraPosition)) {
        ToRemove = true;
        return;
    }
}

sf::FloatRect Podoboo::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 0.f, yPosition() + 0.f), sf::Vector2f(0.f, 0.f));
}

#pragma region Axe

Axe::Axe(sf::Vector2f position) : Enemy(EnemyType::Axe, position) {
    SubPalleteIndex = 3u;

    m_Animate = false;

    Renderer::SetSpriteTheme(1u, 4u);
}

void Axe::OnCollisionWithPlayer(World& world) {
    if (!world.cutscenePlaying()) {
        world.StartCutscene(std::make_unique<AxeScene>(world));
    }
}

sf::FloatRect Axe::getHitbox() const {
    return sf::FloatRect(sf::Vector2f(xPosition() + 3.f, yPosition() + 21.f), sf::Vector2f(11.f, 7.f));
}

#pragma region Lift

Lift::Lift(sf::Vector2f position, bool size) : Enemy(EnemyType::Lift, position), m_Size(4u + size * 2u) {
    SubPalleteIndex = 2u;
    m_Animate = false;
}

void Lift::Update(World& world) {
    if (shouldDespawn(world.CameraPosition)) {
        ToRemove = true;
        return;
    }
}

sf::FloatRect Lift::getHitbox() const {
    return sf::FloatRect(Position, sf::Vector2f(m_Size * 8.f, 8.f));
}

LiftConstant::LiftConstant(sf::Vector2f position, bool size, bool movesDown) : Lift(position, size), m_MovesDown(movesDown) {

}

void LiftConstant::HandleMovement(World&) {

}

LiftOscilating::LiftOscilating(sf::Vector2f position, bool size, bool axis) : Lift(position, size), m_Axis(axis) {
    m_Start = getMovementAxis();
}

void LiftOscilating::HandleMovement(World&) {
    getMovementAxis() = m_Start + OscillatingMovement::getOffset(100u, m_Axis ? 64u : 26u) * (m_Axis ? 1 : -1);
}

LiftFall::LiftFall(sf::Vector2f position, bool size) : Lift(position, size) {

}

void LiftFall::OnPlayerLand(World&) {
    Position.y += 2.f;
    player.Position.y += 2.f;
}

LiftBalance::LiftBalance(sf::Vector2f position) : Lift(position, true) {

}

Enemy* LiftBalance::getFriend(World& world) const noexcept {
    return dynamic_cast<Enemy*>(world.getSprites()[m_FriendSlot].get());
}

void LiftBalance::OnPlayerLand(World& world) {
    if (Position.y >= 176.f) {
        // break lift
        return;
    }

    Position.y += 2.f;
    player.Position.y += 2.f;

    if (const unsigned int row = static_cast<unsigned int>(Position.y / TileSize) - 3) {
        const unsigned int col = static_cast<unsigned int>(Position.x / TileSize) + 1u;
        const unsigned int tileIndex = World::GetIndex(col, row);

        world.m_Tiles[tileIndex] = std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::String_2);
        world.m_AttributeTable[tileIndex] = 1u;
    }

    if (Enemy* friendLift = getFriend(world)) {
        friendLift->Position.y -= 2.f;

        const unsigned int col = static_cast<unsigned int>(friendLift->Position.x / TileSize) + 1u;
        const unsigned int row = static_cast<unsigned int>(friendLift->Position.y / TileSize) - 2;

        world.m_Tiles[World::GetIndex(col, row)].reset();
    }
}