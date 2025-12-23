#include <windows.h>

#include "Sprite/Player.hpp"
#include "World.hpp"
#include "Renderer.hpp"

Player::Player() {
    m_Size = Small;

    m_AcceptPlayerControls = true;
    m_TasMode = false;

    m_SecondPlayerData.Type = 1u;

    Reset();
}

void Player::Swap() {
    std::swap(Data, m_SecondPlayerData);
    Renderer::SetPlayerTheme(Data.Type);
}

void Player::ResetData() {
    Data = PlayerData(0u);
    m_SecondPlayerData = PlayerData(1u);
    Renderer::SetPlayerTheme(0u);
}

void Player::Update(World& world) {
    handlePlayerInput();

    handleCrouching();
    handleJump();
    applyMovement();

    applyFriction();
    applyGravity();

    clampVelocity();
    applyVelocity(world);

    handleFireballShooting(world);

    updateState();
    updateDirection();
    updateInputTimers();

    animateFrame();
    animatePallete();
}

void Player::Reset() {
    m_State = Idle;
    m_Hidden = false;
    m_Visible = true;
    m_Frozen = false;

    m_InvincibilityTimer = 0u;
    m_StarmanTimer = 0u;

    m_OnGround = true;
    m_RisingFromJump = false;

    m_WasRunningBeforeCurrentAction = false;
    m_WasRunningBeforeCurrentAction = false;

    m_Direction = gbl::Direction::Right;
    m_Velocity = sf::Vector2f();

    m_CurrentGravity = 40u;
    m_SwimmingPhysics = false;

    m_AnimationFrame = 0u;
    m_AnimationTimer = getAnimationTimer();
    m_FireballThrowAnimation = false;

    stopPalleteAnimation();
}

#pragma region Properties

void Player::Grow(World& world) {
    if (m_Size == Small) {
        m_Size = Big;

        world.on_otaining_supermushroom();
    } else if (m_Size == Big) {
        m_Size = Fiery;

        Renderer::SetPlayerTheme(2u);

        world.on_otaining_fireflower();
    }
}

bool Player::Damage(World& world) {
    if (m_InvincibilityTimer) {
        return false;
    }

    if (isBig()) {
        if (isFiery()) {
            Renderer::SetPlayerTheme(Data.Type);
        }

        audioPlayer.Play(AudioPlayer::Damage);

        m_Size = Small;

        m_InvincibilityTimer = 8u;

        world.on_player_damage();

        return true;
    } else {
        Kill(world, false);

        return false;
    }
}

void Player::Kill(World& world, bool pit_death) {
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
        if (isFiery()) {
            Renderer::SetPlayerTheme(Data.Type);
        }

        m_Size = Small;

        world.on_player_death(pit_death);
    }
}

void Player::ExtraLife() {
    ++Data.Lives;

    audioPlayer.Play(AudioPlayer::ExtraLife);
}

void Player::StartStarman(World& world) {
    m_StarmanTimer = 35u;
    startPalleteAnimation(2u);

    world.on_otaining_starman();
}

#pragma region Events

void Player::on_side_collision() {
    m_Velocity.x = 0.f;

    if (m_State == Stopping) {
        m_State = Idle;
    }
}

void Player::on_head_collision() {
    endJump();
}

void Player::on_feet_collision() {
    m_Velocity.y = 0.f;
    m_OnGround = true;

    if (m_State == Climbing) {
        m_AnimationFrame = 0u;
    }
}

void Player::OnFramerule(World& world) {
    if (m_StarmanTimer) {
        --m_StarmanTimer;

        if (m_StarmanTimer == 0u) {
            stopPalleteAnimation();
        } else if (m_StarmanTimer == 3u) {
            world.StartThemeMusic();
        } else if (m_StarmanTimer == StarmanSlowdownThreshold) {
            m_PalleteAnimationTimerDuration = 8u;
        }
    }

    if (m_InvincibilityTimer && --m_InvincibilityTimer == 0u) {
        m_Visible = true;
    }
}

#pragma region Input

void Player::resetInputs() {
    m_SprintKeyHeld = 0u;
    m_LeftKeyHeld = false;
    m_RightKeyHeld = false;
    m_UpKeyHeld = false;
    m_DownKeyHeld = false;
    m_JumpKeyHeld = false;
    m_JumpKeyHeldLastFrame = false;
}

void Player::keyboardControl() {
    m_LeftKeyHeld = (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left)
    );

    m_RightKeyHeld = (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)
    );

    m_UpKeyHeld = (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)
    );

    m_DownKeyHeld = (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LShift) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)
    );

    m_JumpKeyHeld = (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)
    );

    if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl) ||
        GetKeyState(VK_CAPITAL) & 0x0001
    ) {
        m_SprintKeyHeld = SprintBufferLength;
    }
}

void Player::joystickControl(uint8_t index) {
    constexpr float AxisThreshold = 15.f;

    float xAxis = sf::Joystick::getAxisPosition(index, sf::Joystick::Axis::X);

    m_LeftKeyHeld = xAxis < -AxisThreshold;
    m_RightKeyHeld = xAxis > AxisThreshold;

    float yAxis = sf::Joystick::getAxisPosition(index, sf::Joystick::Axis::Y);

    m_UpKeyHeld = yAxis < -AxisThreshold;
    m_DownKeyHeld = yAxis > AxisThreshold;

    constexpr uint8_t Joystick_Button_A = 0u;
    constexpr uint8_t Joystick_Button_B = 1u;

    m_JumpKeyHeld = sf::Joystick::isButtonPressed(index, Joystick_Button_A);

    if (sf::Joystick::isButtonPressed(index, Joystick_Button_B)) {
        m_SprintKeyHeld = SprintBufferLength;
    }
}

void Player::handlePlayerInput() {
    if (!m_TasMode && m_AcceptPlayerControls) {
        if (sf::Joystick::isConnected(0u)) {
            joystickControl(0);
        } else {
            keyboardControl();
        }
    }
}

void Player::updateInputTimers() {
    if (!m_Frozen) {
        if (m_SprintKeyHeld) {
            --m_SprintKeyHeld;
        }
    }
}

void Player::on_key_press_left() {
    if (m_OnGround) {
        // on ground and moving right
        if (m_Velocity.x > 0.f) {
            if (m_State != Stopping) {
                m_WasRunningBeforeCurrentAction = runningAtFullSpeed();
            }

            m_State = Stopping;
            m_Direction = gbl::Direction::Left;
        } else if (m_State != Walking) {
            m_State = Walking;
            m_AnimationFrame = 2u;
        }
    }

    // accelerate
    if (m_State != Stopping) {
        m_Velocity.x -= getAcceleration();
    }
}

void Player::on_key_press_right() {
    if (m_OnGround) {
        // on ground and moving left
        if (m_Velocity.x < 0.f) {
            if (m_State != Stopping) {
                m_WasRunningBeforeCurrentAction = runningAtFullSpeed();
            }

            m_State = Stopping;
            m_Direction = gbl::Direction::Right;
        } else if (m_State != Walking) {
            m_State = Walking;
            m_AnimationFrame = 2u;
        }
    }

    // accelerate
    if (m_State != Stopping) {
        m_Velocity.x += getAcceleration();
    }
}

#pragma region Physics

float Player::getAcceleration() const {
    constexpr float AccelerationWalk = 152.f / 256.f / 16.f;
    constexpr float AccelerationSprint = 228.f / 256.f / 16.f;

    if (
        !m_SwimmingPhysics &&
        m_OnGround ? isSprinting() : m_WasRunningBeforeCurrentAction
    ) {
        return AccelerationSprint * (facingOppositeDirectionOfMovement() + 1u);
    } else {
        return AccelerationWalk * (facingOppositeDirectionOfMovement() + 1u);
    }
}

float Player::getMaxSpeed() const {
    if (m_SwimmingPhysics) {
        return m_OnGround ? MaxWalkingSpeedSwimming : MaxWalkingSpeed;
    } else {
        return (m_OnGround ? isSprinting() : m_WasRunningBeforeCurrentAction) ? MaxRunningSpeed : MaxWalkingSpeed;
    }
}

uint8_t Player::getRisingGravity() const {
    if (m_SwimmingPhysics) {
        return 13u;
    }

    if (stillOrWalkingSlowly()) {
        return 32u;
    } else if (walkingAtFullSpeed()) {
        return 30u;
    } else {
        return 40u;
    }
}

uint8_t Player::getFallingGravity() const {
    if (m_SwimmingPhysics) {
        return 10u;
    }

    if (stillOrWalkingSlowly()) {
        return 112u;
    } else if (walkingAtFullSpeed()) {
        return 96u;
    } else {
        return 144u;
    }
}

void Player::applyFriction() {
    if (m_OnGround && m_Velocity.x && !m_Frozen && (sideButtonsNotPressed() || m_State == Stopping || m_DownKeyHeld)) {
        float friction = 152.f / 256.f / 16.f;

        if (m_State == Stopping) {
            if (m_WasRunningBeforeCurrentAction) {
                friction = 416.f / 256.f / 16.f;
            } else {
                friction *= 2.f;
            }
        }

        if (m_Velocity.x > 0.f) {
            m_Velocity.x = std::max(m_Velocity.x - friction, 0.f);
        } else if (m_Velocity.x < 0.f) {
            m_Velocity.x = std::min(m_Velocity.x + friction, 0.f);
        }
    }
}

void Player::applyGravity() {
    if (!m_Frozen) {
        m_Velocity.y += static_cast<float>(m_CurrentGravity) / 256.f;

        // cut off jump (naturally)
        if (m_RisingFromJump && m_Velocity.y >= 0.f) {
            endJump();
        }
    }
}

void Player::applyMovement() {
    if (!m_Frozen) {
        if (!m_DownKeyHeld && m_RightKeyHeld + m_LeftKeyHeld == 1u) {
            if (m_LeftKeyHeld) {
                on_key_press_left();
            } else if (m_RightKeyHeld) {
                on_key_press_right();
            }
        }

        if (m_Velocity.x && currentAbsoluteSpeed() <= 0.625f) {
            if (m_State == Stopping) {
                float resultingVelocity = isSprinting() ? 0.125f : (m_Direction == gbl::Direction::Left ? 0.0625f : 0.f);
                m_Velocity.x = (resultingVelocity + FrictionNormal) * (m_Direction ? 1 : -1);

                m_State = Walking;
                m_AnimationFrame = 2u;
            } else if (isSprinting() && sideButtonsNotPressed() && m_OnGround) {
                m_Velocity.x = 0.f;
            }
        }
    }
}

void Player::applyVelocity(World& world) {
    if (!m_Frozen) {
        Position += m_Velocity;

        bool onGroundLastFrame = m_OnGround;

        world.resolvePlayerTileCollisions();

        if (m_SwimmingPhysics && m_Velocity.y < 0.f && yPosition() < 20.f) {
            m_Velocity.y = 0.f;
        }

        if (onGroundLastFrame != m_OnGround) {
            if (m_OnGround) {
                // landed this frame
                if (facingOppositeDirectionOfMovement()) {
                    m_State = Stopping;
                }
            } else {
                // walked off this frame
                if (m_State == Stopping || m_State == Idle) {
                    m_State = Walking;
                    m_AnimationFrame = 2u;
                }

                m_WasRunningBeforeCurrentAction = runningAtFullSpeed();
            }
        }
    }
}

void Player::clampVelocity() {
    if (m_State != Stopping) {
        const float MaxSpeed = getMaxSpeed();

        if (m_Velocity.x < 0.f) {
            m_Velocity.x = std::max(m_Velocity.x, -MaxSpeed);
        } else {
            m_Velocity.x = std::min(m_Velocity.x, MaxSpeed);
        }
    }

    if (m_Velocity.y > 4.f) {
        m_Velocity.y = 4.f;
    }
}

#pragma region Jumping

void Player::startJump() {
    if (m_OnGround || m_SwimmingPhysics || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::J)) {
        bool running = runningAtFullSpeed();

        m_WasRunningBeforeCurrentAction = running;

        // used to track if in air because of a jump
        m_RisingFromJump = true;

        m_CurrentGravity = getRisingGravity();

        m_OnGround = false;

        if (m_SwimmingPhysics) {
            m_Velocity.y = -2.f;

            m_State = Swimming;

            m_AnimationFrame = 1u;

            audioPlayer.Play(AudioPlayer::Swim);
        } else {
            m_Velocity.y = running ? -5.f : -4.f;

            if (!isCrouching()) {
                m_State = Jumping;
            }

            m_AnimationFrame = 0u;

            audioPlayer.Play(AudioPlayer::PlayerJump1 + isBig());
        }
    }
}

void Player::endJump() {
    m_RisingFromJump = false;
    m_CurrentGravity = getFallingGravity();
}

void Player::handleJump() {
    if (!m_JumpKeyHeld) {
        if (m_RisingFromJump) {
            endJump();
        }
    } else if (!m_JumpKeyHeldLastFrame) {
        startJump();
    }

    m_JumpKeyHeldLastFrame = m_JumpKeyHeld;
}

#pragma region Fireball

void Player::handleFireballShooting(World& world) {
    if (!m_Frozen) {
        const bool sprintPressedThisFrame = m_SprintKeyHeld == SprintBufferLength;

        if (m_Size == Fiery && sprintPressedThisFrame && !m_SprintKeyHeldLastFrame) {
            if (world.SpawnFireball(sf::Vector2f(Position.x, Position.y + 2.f), m_Direction)) {
                m_AnimationTimer = getAnimationTimer();
                m_FireballThrowAnimation = true;

                audioPlayer.Play(AudioPlayer::FireballThrow);
            }
        }

        m_SprintKeyHeldLastFrame = sprintPressedThisFrame;
    }
}

#pragma region Direction

void Player::updateDirection() {
    // only change when mario is on ground and is actually moving and not sliding
    if (!m_Frozen) {
        if (m_SwimmingPhysics) {
            if (m_LeftKeyHeld) {
                m_Direction = gbl::Direction::Left;
            } else if (m_RightKeyHeld) {
                m_Direction = gbl::Direction::Right;
            }
        } else if (m_OnGround && m_Velocity.x && m_State != Stopping) {
            m_Direction = m_Velocity.x > 0.f && (!m_RightKeyHeld || !m_LeftKeyHeld);
        }
    }
}

#pragma region State

void Player::handleCrouching() {
    if (m_OnGround) {
        if (m_DownKeyHeld && sideButtonsNotPressed()) {
            if (isVisualyBig()) {
                m_State = Crouching;
            }
        } else if (isCrouching()) {
            if (sideButtonsNotPressed()) {
                m_State = Idle;
            } else {
                m_State = Walking;
                m_AnimationFrame = 2u;
            }
        }
    }
}

void Player::updateState() {
    if (!m_Frozen && m_OnGround && m_State != Stopping && m_State != Dead && !isCrouching()) {
        if (m_Velocity.x == 0.f && (m_DownKeyHeld || sideButtonsNotPressed())) {
            m_State = Idle;
        } else if (m_State != Walking) {
            m_State = Walking;
            m_AnimationFrame = 2u;
        }
    }
}

#pragma region Animation

uint8_t Player::getAnimationFrames() const {
    if (m_State == Walking) {
        return 3u;
    }

    else if (m_State == Climbing) {
        return 2u;
    }

    else if (m_State == Swimming) {
        return 5u;
    }

    else {
        return 1u;
    }
}

uint8_t Player::getAnimationTimer() const {
    if (m_State == Climbing) {
        return 4u;
    }

    uint8_t speed = static_cast<uint8_t>(currentAbsoluteSpeed() * 16.f);

    if (speed > 27u && !m_SwimmingPhysics) {
        return 2u;
    } else if (speed > 13u) {
        return 4u;
    } else {
        return 7u;
    }
}

void Player::animateFrame() {
    if (!m_Frozen) {
        if (--m_AnimationTimer == 0) {
            m_AnimationTimer = getAnimationTimer();
            m_FireballThrowAnimation = false;

            if (
                m_State == Walking && m_OnGround ||
                m_State == Swimming && (m_AnimationFrame || m_Velocity.y < 0.f) ||
                m_State == Climbing && m_Velocity.y
            ) {
                ++m_AnimationFrame;
            }
        }

        m_AnimationFrame %= getAnimationFrames();
    }

    if (m_InvincibilityTimer) {
        m_Visible ^= true;
    }
}

#pragma region Sub Pallete

void Player::startPalleteAnimation(uint8_t duration) {
    m_AnimatePallete = true;

    m_PalleteAnimationTimerDuration = duration;
    m_PalleteAnimationTimer = duration;
}

void Player::stopPalleteAnimation() {
    m_AnimatePallete = false;
    SubPalleteIndex = 0u;
}

void Player::animatePallete() {
    if (m_AnimatePallete && --m_PalleteAnimationTimer == 0u) {
        m_PalleteAnimationTimer = m_PalleteAnimationTimerDuration;
        SubPalleteIndex = (SubPalleteIndex + 1u) % 4u;
    }
}

// global player instance defination
Player player;

void uint24_t::operator=(uint32_t val) noexcept {
    operator+=(val);
}

void uint24_t::operator+=(uint32_t val) noexcept {
    uint8_t addend[3u]{};

    for (int digitIndex = 5; digitIndex >= 0; --digitIndex) {
        uint8_t digit = val % 10u;
        val /= 10u;

        uint8_t byteIndex = (5u - digitIndex) / 2u;
        uint8_t nibblePos = (5u - digitIndex) % 2u;

        addend[2u - byteIndex] |= nibblePos == 0u ? digit : (digit << 4u);
    }

    bool carry = false;
    for (int i = 2; i >= 0; --i) {
        uint8_t low = (m_Values[i] & 0x0Fu) + (addend[i] & 0x0Fu) + carry;
        if (carry = (low >= 10u)) {
            low -= 10;
        }

        uint8_t high = (m_Values[i] >> 4u) + (addend[i] >> 4u) + carry;
        if (carry = (high >= 10u)) {
            high -= 10;
        }

        m_Values[i] = static_cast<uint8_t>((high << 4u) | low);
    }
}

uint24_t::operator uint32_t() const noexcept {
    uint32_t value = 0u;

    for (uint8_t i = 0u; i < 3u; ++i) {
        value = value * 100u + ((m_Values[i] >> 4u) * 10u + (m_Values[i] & 0x0Fu));
    }

    return value;
}