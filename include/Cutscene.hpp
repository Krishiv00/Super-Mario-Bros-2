#ifndef CUTSCENE_HPP
#define CUTSCENE_HPP

#include "Globals.hpp"

class World;
class Flag;

class Cutscene {
private:
    bool m_WasAcceptingPlayerControls;

protected:
    static constexpr inline const float PlayerMaxAutowalkSpeed = 0.75f;

    static void freezePlayer();
    static void unFreezePlayer();

    static void killPlayer();

    static void landPlayer();
    static void unCrouchPlayer();

    static void hidePlayer();
    static void unHidePlayer();

    static void stopPlayer();
    static void setPlayerYVel(float vel);
    static void setPlayerXVel(float vel);

    void stepPlayerMovement() const;
    static void stepPlayerAnimation();

    static void onPlayerLeaveFlag();

    static void enablePlayerControls();
    static void disablePlayerControls();

    static void handlePlayerAutowalk();
    static void stopPlayerAutowalk();

    void lockScroll() const;

    void freezeWorld() const;
    void unFreezeWorld() const;

    void freezeGame() const;
    void unFreezeGame() const;

    void enableCollision(bool y_axis) const;
    void disableCollision(bool y_axis) const;

    void enableAutoScroll() const;
    void disableAutoScroll() const;

    void startLevel(uint8_t newWorld, uint8_t newLevel, bool levelTransition) const;
    void restartLevel() const;

    void moveFlagNum() const;

    void timerCountDown() const;

    static void bouncePlayer();

    static void setPlayerFrame(uint8_t frame);
    static void setPlayerState(uint8_t state);
    static void setPlayerVisible(bool value);
    static void setPlayerDirection(bool dir);

    static void startPlayerPalleteAnimation(uint8_t duration);
    static void stopPlayerPalleteAnimation();

    World& m_WorldRef;

public:
    Cutscene(World& world);
    virtual ~Cutscene();

    virtual void Update() = 0;
    virtual void OnFramerule() {}
    virtual bool EndScene() = 0;
};

class TimedCutscene : public Cutscene {
protected:
    uint8_t m_Timer;

public:
    TimedCutscene(World& world, uint8_t initialTimer) : Cutscene(world), m_Timer(std::move(initialTimer)) {}

    virtual void Update() override {
        --m_Timer;
    }

    virtual bool EndScene() override {
        return m_Timer == 0u;
    }
};

class SizeChangingScene : public TimedCutscene {
protected:
    uint8_t m_PlayerLastState;

public:
    SizeChangingScene(World& world);
    ~SizeChangingScene();
};

class GrowingScene : public SizeChangingScene {
public:
    GrowingScene(World& world);

    virtual void Update() override;
};

class ShrinkingScene : public SizeChangingScene {
private:
    uint8_t m_CameraMoveTimer = 0u;

public:
    ShrinkingScene(World& world);

    virtual void Update() override;
};

class FireFlowerScene : public TimedCutscene {
public:
    FireFlowerScene(World& world);
    ~FireFlowerScene();
};

class DeathScene : public Cutscene {
protected:
    uint8_t m_Timer; // framerules

public:
    DeathScene(World& world);
    ~DeathScene();

    virtual void Update() override {}
    virtual void OnFramerule() override;

    virtual bool EndScene() override;
};

class BounceDeathScene : public DeathScene {
private:
    uint8_t m_BounceTimer = 16u;

public:
    BounceDeathScene(World& world);
    ~BounceDeathScene();

    virtual void Update() override;
};

class LevelClearScene : public Cutscene {
protected:
    uint8_t m_LevelEndTimer;

public:
    LevelClearScene(World& world);
    ~LevelClearScene();

    virtual bool EndScene() override {
        return false;
    };
};

class FlagpoleScene : public LevelClearScene {
private:
    enum class Routine : uint8_t {
        None,
        SlidingDown,
        TurnAround,
        LeaveFlag,
        WalkToCastle,
        TimerCountdown
    };

    void handleRoutineSlidingDown();
    void handleRoutineTurnAround();
    void handleRoutineLeaveFlag();
    void handleRoutineWalkToCastle();
    void handleRoutineTimerCountdown();

    Routine m_CurrentRoutine;
    uint8_t m_RoutineTimer;
    uint8_t m_LevelClearTimer;
    Flag* m_FlagRef;

public:
    FlagpoleScene(World& world);
    ~FlagpoleScene();

    virtual void Update() override;
    virtual void OnFramerule() override;
};

class AxeScene : public LevelClearScene {
private:

public:
    AxeScene(World& world);
    ~AxeScene();

    virtual void Update() override;
};

class LPipeScene : public LevelClearScene {
private:
    uint8_t m_AutowalkTimer;

public:
    LPipeScene(World& world);
    ~LPipeScene();

    virtual void Update() override;
};

class DPipeScene : public LevelClearScene {
private:

public:
    DPipeScene(World& world);
    ~DPipeScene();

    virtual void Update() override;
};

class AutowalkScene : public Cutscene {
public:
    AutowalkScene(World& world);
    ~AutowalkScene();

    virtual void Update() override;
    virtual bool EndScene() {
        return false;
    };
};

class PlayerSpawnScene : public TimedCutscene {
public:
    PlayerSpawnScene(World& world, uint8_t posIdx);
    ~PlayerSpawnScene();
};

#endif // !CUTSCENE_HPP