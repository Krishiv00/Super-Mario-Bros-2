#include "Cutscene.hpp"
#include "World.hpp"
#include "Sprite/Player.hpp"
#include "Renderer.hpp"

Cutscene::Cutscene(World& world) : m_WorldRef(world) {
    m_WasAcceptingPlayerControls = player.m_AcceptPlayerControls;
    player.m_AcceptPlayerControls = false;
    player.m_Visible = true;
}

Cutscene::~Cutscene() {
    player.m_AcceptPlayerControls = m_WasAcceptingPlayerControls;
}

#pragma region Abstractions

void Cutscene::freezePlayer() {
    player.m_Frozen = true;
}

void Cutscene::unFreezePlayer() {
    player.m_Frozen = false;
}

void Cutscene::killPlayer() {
    --player.m_Lives;
}

void Cutscene::landPlayer() {
    player.m_OnGround = true;
}

void Cutscene::unCrouchPlayer() {
    player.m_State = Player::Walking;
    player.m_DownKeyHeld = false;
    player.m_AnimationFrame = 2u;
}

void Cutscene::hidePlayer() {
    player.m_Hidden = true;
}

void Cutscene::unHidePlayer() {
    player.m_Hidden = false;
}

void Cutscene::stopPlayer() {
    player.m_Velocity = sf::Vector2f();
    player.resetInputs();
}

void Cutscene::setPlayerYVel(float vel) {
    player.m_Velocity.y = vel;
}

void Cutscene::setPlayerXVel(float vel) {
    player.m_Velocity.x = vel;
}

void Cutscene::stepPlayerMovement() const {
    player.m_Frozen = false;
    player.applyVelocity(m_WorldRef);
    player.m_Frozen = true;
}

void Cutscene::stepPlayerAnimation() {
    unFreezePlayer();
    player.animateFrame();
    freezePlayer();
}

void Cutscene::onPlayerLeaveFlag() {
    setPlayerState(Player::Walking);
    setPlayerFrame(2u);
    player.Position.x += 4.f;
    unFreezePlayer();

    musicPlayer.Play(MusicPlayer::Clear_1, false);

    player.m_Velocity.x = 1.1875f;
}

void Cutscene::enablePlayerControls() {
    player.m_AcceptPlayerControls = true;
}

void Cutscene::disablePlayerControls() {
    player.m_AcceptPlayerControls = false;
    player.m_RightKeyHeld = false;
    player.m_LeftKeyHeld = false;
}

void Cutscene::handlePlayerAutowalk() {
    player.m_RightKeyHeld = true;

    if (player.m_Velocity.x > PlayerMaxAutowalkSpeed) {
        player.m_Velocity.x = PlayerMaxAutowalkSpeed;
    }
}

void Cutscene::stopPlayerAutowalk() {
    player.m_RightKeyHeld = false;
    setPlayerState(Player::Idle);
}

void Cutscene::lockScroll() const {
    m_WorldRef.m_ScrollLocked = true;
}

void Cutscene::freezeWorld() const {
    m_WorldRef.m_Frozen = true;
}

void Cutscene::unFreezeWorld() const {
    m_WorldRef.m_Frozen = false;
}

void Cutscene::freezeGame() const {
    freezeWorld();
    freezePlayer();
}

void Cutscene::unFreezeGame() const {
    unFreezeWorld();
    unFreezePlayer();
}

void Cutscene::enableCollision(bool y_axis) const {
    if (y_axis) {
        m_WorldRef.m_CollisionMode |= World::YCollision;
    } else {
        m_WorldRef.m_CollisionMode |= World::XCollision;
    }
}

void Cutscene::disableCollision(bool y_axis) const {
    if (y_axis) {
        m_WorldRef.m_CollisionMode &= World::XCollision;
    } else {
        m_WorldRef.m_CollisionMode &= World::YCollision;
    }
}

void Cutscene::enableAutoScroll() const {
    m_WorldRef.m_AutoScroll = true;
}

void Cutscene::disableAutoScroll() const {
    m_WorldRef.m_AutoScroll = false;
}

void Cutscene::restartFromCheckpoint() const {

}

void Cutscene::startLevel(uint8_t newLevel, uint8_t newStage, bool levelTransition) const {
    if (levelTransition) {
        m_WorldRef.m_NewLevel = true;
    } else {
        m_WorldRef.m_NewArea = true;
    }

    m_WorldRef.setLevel(newLevel, newStage);
}

void Cutscene::moveFlagNum() const {
    m_WorldRef.m_FloateyNums[World::SpecialSpriteSlot].m_Position.y -= 2;
}

void Cutscene::timerCountDown() const {
    --m_WorldRef.m_GameTime;
    player.m_Score += 50u;

    constexpr uint8_t Delay = 6u;

    if (m_WorldRef.m_GameTime >= Delay) {
        if (m_WorldRef.m_GameTime % Delay == 0u) {
            audioPlayer.Play(AudioPlayer::CoinAcquire);
        }
    } else {
        audioPlayer.StopAll();
    }
}

void Cutscene::bouncePlayer() {
    player.m_Velocity = sf::Vector2f(0.f, -4.f);
    player.m_CurrentGravity = 40u;
}

void Cutscene::setPlayerFrame(uint8_t frame) {
    player.m_AnimationFrame = frame;
}

void Cutscene::setPlayerState(uint8_t state) {
    player.m_State = state;

    setPlayerFrame(0u);
}

void Cutscene::setPlayerVisible(bool value) {
    player.m_Visible = value;
}

void Cutscene::setPlayerDirection(bool dir) {
    player.m_Direction = dir;
}

void Cutscene::startPlayerPalleteAnimation(uint8_t duration) {
    player.startPalleteAnimation(duration);
}

void Cutscene::stopPlayerPalleteAnimation() {
    if (player.HasStarman()) {
        player.startPalleteAnimation(player.m_StarmanTimer <= Player::StarmanSlowdownThreshold ? 8u : 2u);
    } else {
        player.stopPalleteAnimation();
    }
}

#pragma region Size Change

SizeChangingScene::SizeChangingScene(World& world) : TimedCutscene(world, 44u), m_PlayerLastState(player.getState()) {
    landPlayer();
    setPlayerState(Player::Changing);
    freezeGame();
}

SizeChangingScene::~SizeChangingScene() {
    setPlayerState(m_PlayerLastState == Player::Jumping ? Player::Walking : m_PlayerLastState);
    unFreezeGame();
}

GrowingScene::GrowingScene(World& world) : SizeChangingScene(world) {

}

void GrowingScene::Update() {
    TimedCutscene::Update();

    constexpr uint8_t FrameSeries[] = {
        2u, 2u, 1u, 0u, 2u, 1u, 0u, 1u, 0u, 1u, 0u
    };

    setPlayerFrame(FrameSeries[m_Timer / 4u]);
}

ShrinkingScene::ShrinkingScene(World& world) : SizeChangingScene(world) {
    if (player.getVelocity().x > 0.8125f) {
        m_CameraMoveTimer = 16u;
    }
}

void ShrinkingScene::Update() {
    TimedCutscene::Update();

    constexpr uint8_t FrameSeries[] = {
        2u, 1u, 2u, 1u, 2u, 1u, 2u, 1u, 2u, 1u, 0u
    };

    setPlayerFrame(FrameSeries[m_Timer / 4u]);

    if (m_Timer == 0u) {
        disableAutoScroll();
    } else if (m_CameraMoveTimer && --m_CameraMoveTimer == 0u) {
        enableAutoScroll();
    }
}

#pragma region Fire Flower

FireFlowerScene::FireFlowerScene(World& world) : TimedCutscene(world, 64u) {
    freezeGame();
    landPlayer();
    startPlayerPalleteAnimation(4u);
}

FireFlowerScene::~FireFlowerScene() {
    stopPlayerPalleteAnimation();
    unFreezeGame();
}

#pragma region Death

DeathScene::DeathScene(World& world) : Cutscene(world) {
    musicPlayer.Play(MusicPlayer::Death);

    freezePlayer();
    disablePlayerControls();

    disableCollision(0);
    disableCollision(1);

    m_Timer = 12u;
}

DeathScene::~DeathScene() {
    unFreezePlayer();
    enablePlayerControls();

    enableCollision(0);
    enableCollision(1);

    killPlayer();

    startLevel(m_WorldRef.getLevel(), m_WorldRef.getStage(), true);
}

void DeathScene::OnFramerule() {
    --m_Timer;
}

bool DeathScene::EndScene() {
    return m_Timer == 0u;
}

BounceDeathScene::BounceDeathScene(World& world) : DeathScene(world) {
    setPlayerState(Player::Dead);
    bouncePlayer();
    freezeWorld();
}

BounceDeathScene::~BounceDeathScene() {
    setPlayerState(Player::Idle);
    unFreezeWorld();
}

void BounceDeathScene::Update() {
    if (m_BounceTimer && --m_BounceTimer == 0) {
        unFreezePlayer();
    }
}

#pragma region Level Clear

LevelClearScene::LevelClearScene(World& world) : Cutscene(world) {
    disablePlayerControls();
}

LevelClearScene::~LevelClearScene() {
    enablePlayerControls();
}

FlagpoleScene::FlagpoleScene(World& world) : LevelClearScene(world) {
    musicPlayer.Stop();
    audioPlayer.StopAll();

    stopPlayer();
    freezePlayer();

    setPlayerState(Player::Climbing);
    setPlayerDirection(gbl::Direction::Right);

    player.Position.x = std::floor(player.getHitbox().position.x / 16.f) * 16.f + 9.f;

    setPlayerYVel(2.f);

    if (player.yPosition() >= 166.f) { // FFPG
        hidePlayer();
        lockScroll();
        stopPlayer();
        setPlayerState(Player::Walking);
        setPlayerFrame(2u);
        unFreezePlayer();
        musicPlayer.Play(MusicPlayer::Clear_1, false);

        m_CurrentRoutine = Routine::WalkToCastle;
    } else if (player.yPosition() >= 164.f) { // FPG
        m_RoutineTimer = 24u;
        stopPlayer();
        player.Position.y = 160.f;
        setPlayerFrame(0u);

        m_CurrentRoutine = Routine::TurnAround;
    } else {
        if (m_FlagRef = GetIf(world.getSprites()[World::SpecialSpriteSlot].get(), Flag)) {
            m_FlagRef->SetMoving(true);
            musicPlayer.Play(MusicPlayer::Flag, false);
        }

        m_CurrentRoutine = Routine::SlidingDown;
    }
}

FlagpoleScene::~FlagpoleScene() {
    unFreezePlayer();
}

void FlagpoleScene::handleRoutineSlidingDown() {
    stepPlayerAnimation();
    stepPlayerMovement();

    moveFlagNum();

    // end this routine
    if (!m_FlagRef || m_FlagRef->ReachedBottom()) {
        if (m_FlagRef) {
            m_FlagRef->SetMoving(false);
        }

        stopPlayer();

        m_RoutineTimer = 2u;
        m_CurrentRoutine = Routine::TurnAround;
    }
}

void FlagpoleScene::handleRoutineTurnAround() {
    // end this routine
    if (--m_RoutineTimer == 0u) {
        player.Position.x += 14.f;
        setPlayerDirection(gbl::Direction::Left);

        m_RoutineTimer = 24u;
        m_CurrentRoutine = Routine::LeaveFlag;
    }
}

void FlagpoleScene::handleRoutineLeaveFlag() {
    // end this routine
    if (--m_RoutineTimer == 0u) {
        onPlayerLeaveFlag();
        m_CurrentRoutine = Routine::WalkToCastle;
    }
}

void FlagpoleScene::handleRoutineWalkToCastle() {
    handlePlayerAutowalk();

    // end this routine
    if (player.getVelocity().x == 0.f && player.IsOnGround()) {
        hidePlayer();
        lockScroll();
        m_CurrentRoutine = Routine::TimerCountdown;
    }
}

void FlagpoleScene::handleRoutineTimerCountdown() {
    if (m_WorldRef.getGameTime()) {
        timerCountDown();
        handlePlayerAutowalk();
    } else {
        // end this routine
        m_LevelClearTimer = 6u;
        m_CurrentRoutine = Routine::None;
    }
}

void FlagpoleScene::Update() {
    if (m_CurrentRoutine == Routine::SlidingDown) {
        handleRoutineSlidingDown();
    } else if (m_CurrentRoutine == Routine::TurnAround) {
        handleRoutineTurnAround();
    } else if (m_CurrentRoutine == Routine::LeaveFlag) {
        handleRoutineLeaveFlag();
    } else if (m_CurrentRoutine == Routine::WalkToCastle) {
        handleRoutineWalkToCastle();
    } else if (m_CurrentRoutine == Routine::TimerCountdown) {
        handleRoutineTimerCountdown();
    }
}

void FlagpoleScene::OnFramerule() {
    if (--m_LevelClearTimer == 0u) {
        startLevel(m_WorldRef.getLevel(), m_WorldRef.getStage() + 1u, true);
    }
}

AxeScene::AxeScene(World& world) : LevelClearScene(world) {
    musicPlayer.Stop();

    freezeGame();
}

AxeScene::~AxeScene() {
    unFreezeGame();
}

void AxeScene::Update() {
    startLevel(m_WorldRef.getLevel() + 1u, 1u, true);
}

#pragma region Warp

LPipeScene::LPipeScene(World& world) : LevelClearScene(world) {
    hidePlayer();
    stopPlayer();

    setPlayerState(Player::Walking);

    float endPos = std::truncf(player.getHitbox().position.x / 16.f + 1.f) * 16.f + 2.f;
    m_AutowalkTimer = (endPos - player.xPosition()) / PlayerMaxAutowalkSpeed;

    m_LevelEndTimer = 48u;

    audioPlayer.Play(AudioPlayer::Pipe);
}

LPipeScene::~LPipeScene() {
    unHidePlayer();
}

void LPipeScene::Update() {
    if (m_AutowalkTimer) {
        if (--m_AutowalkTimer == 0u) {
            stopPlayerAutowalk();
        } else {
            handlePlayerAutowalk();
        }
    } else if (--m_LevelEndTimer == 0u) {
        startLevel(m_WorldRef.getLevel(), m_WorldRef.getStage() + 1u, true);
    }
}

DPipeScene::DPipeScene(World& world) : LevelClearScene(world) {
    freezePlayer();
    hidePlayer();
    disablePlayerControls();

    if (player.getState() == Player::Jumping) {
        setPlayerState(std::fabs(player.getVelocity().x) ? Player::Walking : Player::Idle);
    }

    if (player.getVelocity().x > 0.8125f) {
        enableAutoScroll();
    }

    m_LevelEndTimer = 48u;

    audioPlayer.Play(AudioPlayer::Pipe);
}

DPipeScene::~DPipeScene() {
    enablePlayerControls();
    m_WorldRef.TickDownTimer();
}

void DPipeScene::Update() {
    if (--m_LevelEndTimer == 0u) {
        startLevel(m_WorldRef.getLevel(), m_WorldRef.getStage(), false);
        return;
    }

    ++player.Position.y;

    if (player.getState() == Player::Walking) {
        stepPlayerAnimation();
    } else {
        setPlayerFrame(0u);
    }
}

#pragma region Autowalk

AutowalkScene::AutowalkScene(World& world) : Cutscene(world) {
    disablePlayerControls();
}

AutowalkScene::~AutowalkScene() {
    enablePlayerControls();
}

void AutowalkScene::Update() {
    handlePlayerAutowalk();
}

#pragma region Player Spawn

PlayerSpawnScene::PlayerSpawnScene(World& world, uint8_t posIdx) : TimedCutscene(world, 18u * (2u - posIdx)) {
    Renderer::SetGameTimeRendering(false);

    unCrouchPlayer();
    disablePlayerControls();
}

PlayerSpawnScene::~PlayerSpawnScene() {
    Renderer::SetGameTimeRendering(true);

    enablePlayerControls();
}