#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Globals.hpp"
#include "SFML/Graphics.hpp"

#include "Audio.hpp"

#include "Sprite/Sprite.hpp"

namespace EnemyType {
    enum Type : uint8_t {
        Goomba,
        KoopaTroopa,
        KoopaTroopaShell,
        PiranhaPlant,
        KoopaParatroopa,
        BuzzyBeetle,
        BuzzyBeetleShell,
        SpinyEgg,
        Spiny,
        Bloober,
        CheepCheep,
        Lakitu,
        HammerBrother,
        BulletBill,
        DeadGoomba,
        Firebar,
        Podoboo,
        Axe,
        Lift,
        NPC,
        Flag,

        // enemies which are actually different but uses the same texture in the renderer
        RedKoopaTroopa = 0x80 | KoopaTroopa,
        RedKoopaShell = 0x80 | KoopaTroopaShell,
        RedKoopaParatroopa = 0x80 | KoopaParatroopa,
    };
}

namespace EnemyComponents {
    class Shell;
}

class Enemy : public Sprite {
    friend class Renderer;
    friend class MapLoader;
    friend class World;
    friend class LiftBalance;
    friend class EnemyComponents::Shell;

protected:
    void spawnDeathAnimation(World& world, int8_t direction, float initialVelocity) noexcept;

    void givePlayerScore(uint16_t score, World& world) noexcept;
    void givePlayerLife(World& world) noexcept;

    void setDirectionRelativeToPlayer() noexcept;

    [[nodiscard]] bool shouldDespawn(float cameraPosition, float maxThreshold) const noexcept;

    virtual void onCollide(World& world);
    virtual void onBlockDefeat(World& world, float blockPosition);
    virtual void onFireballDeath(World& world, int8_t fireballDirection);
    void onShellDeath(World& world, uint8_t killChain, int8_t shellDirection) noexcept;

    int8_t m_Direction = -1;

    EnemyType::Type m_Type;

    bool m_Animate = true;

    uint8_t SlotIndex = 0;

    bool m_TouchingPlayer = true;

    Enemy() = default;
    Enemy(EnemyType::Type type, sf::Vector2f position);

public:
    virtual void OnFramerule(World&) {}

    virtual void OnCollisionWithPlayer(World& world);
    void OnNoCollision() noexcept;

    virtual void HandleMovement(World&) {}

    [[nodiscard]] virtual sf::FloatRect getHitbox() const {
        return sf::FloatRect(sf::Vector2f(), sf::Vector2f());
    }
};

namespace EnemyComponents {
    class SideToSideMovement : virtual public Enemy {
    protected:
        void update(World& world, float speed, bool make_sound);
    };

    class GravityMovement : virtual public Enemy {
    protected:
        bool m_OnGround = false;
        float m_YVelocity = 0.f;

        void update(World& world, float gravityForce);
    };

    class CollideWithOtherEnemies : virtual public Enemy {
    protected:
        void update(World& world);
    };

    class Stompable : virtual public Enemy {
    public:
        virtual void onStomp(World& world);

        bool isStomped();
    };

    class GroundEnemy : public EnemyComponents::SideToSideMovement, public EnemyComponents::GravityMovement, public EnemyComponents::CollideWithOtherEnemies {
    public:
        virtual void HandleMovement(World& world) override;
        virtual void Update(World& world) override;
    };

    class ShellEnemy : public EnemyComponents::GroundEnemy, public EnemyComponents::Stompable {
        friend class Shell;

    private:
        std::unique_ptr<Shell> getShellObj() const;

        virtual void onStomp(World& world) override;
        virtual void onBlockDefeat(World& world, float blockPosition);
    };

    class Shell : public EnemyComponents::SideToSideMovement, public EnemyComponents::GravityMovement, public EnemyComponents::CollideWithOtherEnemies, public EnemyComponents::Stompable {
        friend class ShellEnemy;

    private:
        void setMovingState();
        void setRestState();

        void giveKickPoints(World& world);

        void killOtherEnemies(World& world);

        virtual void onStomp(World& world) override;
        virtual void onCollide(World& world) override;

        virtual void OnFramerule(World& world) override;

        bool m_Moving;
        bool m_FlippedVertically;
        uint8_t m_RevivalTimer;
        uint8_t m_KillChain;

    public:
        Shell();

        virtual void HandleMovement(World& world) override;
        virtual void Update(World& world) override;

        [[nodiscard]] inline const bool& IsFlipped() const noexcept {
            return m_FlippedVertically;
        }
    };

    class OscillatingMovement {
    private:
        uint8_t m_Phase = 0u;
        uint8_t m_Frame = 0u;

    protected:
        [[nodiscard]] float getOffset(uint8_t phaseDuration, uint8_t halfwayDistance);
    };

    class BowserPart : public Enemy {

    };
}

class Goomba final : public EnemyComponents::GroundEnemy, public EnemyComponents::Stompable {
public:
    Goomba(sf::Vector2f position);

    virtual void onStomp(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class DeadGoomba final : public EnemyComponents::GravityMovement, public EnemyComponents::CollideWithOtherEnemies {
private:
    virtual void onCollide(World&) override {}

    uint8_t m_Timer;

public:
    DeadGoomba(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class KoopaTroopa : public EnemyComponents::ShellEnemy {
    friend class KoopaParatroopa;
    friend class RedKoopaParatroopa;

protected:
    KoopaTroopa() = default;

public:
    KoopaTroopa(sf::Vector2f position);

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class KoopaTroopaShell : public EnemyComponents::Shell {
protected:
    KoopaTroopaShell() = default;

public:
    KoopaTroopaShell(sf::Vector2f position);

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class KoopaParatroopa final : public EnemyComponents::SideToSideMovement, public EnemyComponents::GravityMovement, public EnemyComponents::CollideWithOtherEnemies, public EnemyComponents::Stompable {
private:
    virtual void onStomp(World& world) override;

public:
    KoopaParatroopa(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class RedKoopaTroopa final : public KoopaTroopa {
public:
    RedKoopaTroopa(sf::Vector2f position);

    virtual void Update(World& world) override;
};

class RedKoopaShell final : public KoopaTroopaShell {
public:
    RedKoopaShell(sf::Vector2f position) : Enemy(EnemyType::RedKoopaShell, position) {}
};

class RedKoopaParatroopa final : public EnemyComponents::Stompable, public EnemyComponents::OscillatingMovement {
private:
    virtual void onStomp(World& world) override;

    float m_Starty;

public:
    RedKoopaParatroopa(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class PiranhaPlant final : public Enemy {
private:
    uint8_t m_PauseTimer;
    uint8_t m_MovementTimer;

public:
    PiranhaPlant(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class BuzzyBeetle final : public EnemyComponents::ShellEnemy {
private:
    virtual void onFireballDeath(World& world, int8_t fireballDirection) override;

public:
    BuzzyBeetle(sf::Vector2f position);

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class BuzzyBeetleShell final : public EnemyComponents::Shell {
private:
    virtual void onFireballDeath(World& world, int8_t fireballDirection) override;

public:
    BuzzyBeetleShell(sf::Vector2f position);

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class SpinyEgg final : public EnemyComponents::GravityMovement {
public:
    SpinyEgg(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class Spiny final : public EnemyComponents::GroundEnemy {
    friend class SpinyEgg;

public:
    Spiny(sf::Vector2f position);

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class Bloober final : public EnemyComponents::Stompable {
public:
    Bloober(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class CheepCheep final : public EnemyComponents::Stompable {
public:
    CheepCheep(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class HammerBrother final : public EnemyComponents::GravityMovement, public EnemyComponents::Stompable {
public:
    HammerBrother(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class Lakitu final : public EnemyComponents::Stompable {
public:
    Lakitu(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class BulletBill final : public EnemyComponents::Stompable {
public:
    BulletBill(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class Firebar final : public Enemy {
    friend class Renderer;

private:
    void handleCollision(World& world);

    const bool m_Speed;

    const uint8_t m_Size;
    uint8_t m_Angle;

    bool m_MoveState;
    uint8_t m_MoveTimer;

public:
    Firebar(sf::Vector2f position, bool size, bool direction, bool fast);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] float getAngle() const {
        return toRad(static_cast<float>(m_Angle) * 11.25f);
    }
};

class Podoboo final : public Enemy {
private:

public:
    Podoboo(sf::Vector2f position);

    virtual void HandleMovement(World& world) override;
    virtual void Update(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class Axe final : public Enemy {
private:

public:
    Axe(sf::Vector2f position);

    virtual void Update(World&) override {}
    virtual void OnCollisionWithPlayer(World& world) override;

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class Lift : public Enemy {
    friend class Renderer;

private:
    const uint8_t m_Size;

public:
    Lift(sf::Vector2f position, bool size);

    virtual void Update(World& world) override;

    virtual void OnCollisionWithPlayer(World&) override {}

    virtual void OnPlayerLand(World&) {}

    [[nodiscard]] virtual sf::FloatRect getHitbox() const override;
};

class LiftConstant final : public Lift {
private:
    bool m_MovesDown;

public:
    LiftConstant(sf::Vector2f position, bool size, bool movesDown);

    virtual void HandleMovement(World& world) override;
};

class LiftOscilating final : public Lift, public EnemyComponents::OscillatingMovement {
    friend class MapLoader;

private:
    float& getMovementAxis() {
        return m_Axis ? Position.y : Position.x;
    }

    bool m_Axis;

    float m_Start;

public:
    LiftOscilating(sf::Vector2f position, bool size, bool axis);

    virtual void HandleMovement(World& world) override;
};

class LiftFall final : public Lift {
public:
    LiftFall(sf::Vector2f position, bool size);

    virtual void OnPlayerLand(World& world) override;
};

class LiftBalance final : public Lift {
    friend class World;

private:
    Enemy* getFriend(World& world) const noexcept;

    uint8_t m_FriendSlot;

public:
    LiftBalance(sf::Vector2f position);

    virtual void OnPlayerLand(World& world) override;
};

class NPC : public Enemy {
public:
    NPC(sf::Vector2f position, bool type);

    virtual void Update(World&) override {}
};

class BowserPart_1 final : public EnemyComponents::BowserPart {

};

class BowserPart_2 final : public EnemyComponents::BowserPart {
    
};

#endif // !ENEMY_HPP