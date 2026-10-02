#include "World.hpp"
#include "Renderer.hpp"

#pragma region Reset

void World::Reset() {
    initGameTime(0u);

    m_MultiCoinTimer = 0u;
    m_MulticoinTimerActive = false;

    m_Tiles.clear();
    m_AttributeTable.clear();
    m_SpritePool.clear();

    m_CollisionMode = XCollision | YCollision;

    for (auto& slot : m_Sprites) {
        slot.reset();
    }

    for (auto& ball : m_Fireballs) {
        ball.reset();
    }

    for (auto& hammer : m_Hammers) {
        hammer.reset();
    }

    for (FloateyNum& floateyNum : m_FloateyNums) {
        floateyNum.Reset();
    }

    for (auto& sprite : m_BouncingCoins) {
        sprite.reset();
    }

    for (auto& sprite : m_Fireworks) {
        sprite.reset();
    }

    for (auto& sprite : m_DeathAnimations) {
        sprite.reset();
    }

    m_CheckEnemyCollisions = false;
    m_AutoScroll = false;
    m_ScrollLocked = false;

    m_Frozen = false;

    m_BumpTimer = 0u;
    m_BouncingBlock.reset();

    m_Cutscene.reset();

    m_NewLevel = false;
    m_NewArea = false;

    m_StompChain = 0u;

    CameraPosition = 0.f;
}

#pragma region Update

void World::Update() {
    ++FrameCounter;

    if (m_Cutscene) {
        m_Cutscene->Update();

        if (m_Cutscene->EndScene()) {
            stopCutscene();
        }
    }

    if (m_Frozen) {
        updateGrowingPowerup();
    } else {
        handlePowerupCollisions();
        updateSprites();

        handleSpriteLoading();
    }

    updateFreezeIndependentSprites();

    updateTimers();

    if (!scrollLocked()) {
        moveCamera();
    }

    clampPlayerLeft();
}

void World::OnFramerule() {
    if (m_Cutscene) {
        m_Cutscene->OnFramerule();
    }

    for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
        if (auto& slot = m_Sprites[i]) {
            if (Enemy* enemy = GetIf(slot.get(), Enemy)) {
                enemy->OnFramerule(*this);
            }
        }
    }

    if (m_MultiCoinTimer) {
        --m_MultiCoinTimer;
    }
}

void World::TickDownTimer() {
    m_GameTimeUpdateTimer = GameTimerUpdateLength;

    if (--m_GameTime == 0u) {
        player.Kill(*this, false);
    }
}

bool World::PointInTile(sf::Vector2f point) const {
    const unsigned int col = static_cast<unsigned int>(point.x / TileSize);
    const unsigned int row = static_cast<unsigned int>(point.y / TileSize) - 2;

    const Blocks::Block* block = m_Tiles[World::GetIndex(col, row)].get();

    return block && HasComponent(block, Components::Collision);
}

#pragma region Events

void World::on_block_hit_from_bottom(unsigned int x, unsigned int y) {
    updateBouncingBlock();

    auto& block = m_Tiles[World::GetIndex(x, y)];

    if (const Components::Item* itemComponent = GetComponent(block.get(), const Components::Item)) {
        const uint8_t itemId = itemComponent->ItemId;
        bool use = true;

        if (gbl::ItemType::isPowerupItem(itemId)) {
            m_Sprites[SpecialSpriteSlot].reset();

            audioPlayer.Play(AudioPlayer::PowerupSpawn);
        } else if (gbl::ItemType::isCoinItem(itemId)) {
            if (
                const Components::Render* renderComponent = GetComponent(block.get(), const Components::Render);
                renderComponent &&
                renderComponent->TextureId != gbl::TextureId::Block::Question
            ) {
                if (m_MultiCoinTimer == 0u) {
                    if (!m_MulticoinTimerActive) {
                        m_MultiCoinTimer = 11u;
                        use = false;
                    }

                    m_MulticoinTimerActive ^= 1;
                } else {
                    use = false;
                }
            }

            give_coin();
            spawnCoinAnimation(x, y + 2u);
        }

        if (use) {
            block = std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Question_used);
        }

        m_BouncingBlock = std::make_unique<BouncingBlock>(block, itemId, m_BumpTimer, x, y, m_AttributeTable[World::GetIndex(x, y)]);

        handleBlockDefeat(m_BouncingBlock->Position);
    } else if (HasComponent(block.get(), Components::Hitable)) {
        m_BouncingBlock = std::make_unique<BouncingBlock>(block, gbl::ItemType::None, m_BumpTimer, x, y, m_AttributeTable[World::GetIndex(x, y)]);

        handleBlockDefeat(m_BouncingBlock->Position);
    }

    audioPlayer.Play(AudioPlayer::BlockHit);
}

void World::on_enter_warp_zone() {
    m_SpawnOneUp = true;
}

void World::on_obtaining_supermushroom() {
    StartCutscene(std::make_unique<GrowingScene>(*this));
}

void World::on_obtaining_fireflower() {
    StartCutscene(std::make_unique<FireFlowerScene>(*this));

    for (auto& ball : m_Fireballs) {
        ball.reset();
    }
}

void World::on_obtaining_starman() {
    musicPlayer.Play(MusicPlayer::Star, true);
}

void World::on_player_damage() {
    StartCutscene(std::make_unique<ShrinkingScene>(*this));
}

void World::on_player_death(bool pit_death) {
    if (pit_death) {
        StartCutscene(std::make_unique<DeathScene>(*this));
    } else {
        StartCutscene(std::make_unique<BounceDeathScene>(*this));
    }
}

void World::StartThemeMusic() {
    musicPlayer.LoadFromFile(MusicPlayer::MainTheme, "Resources/Music/Theme_" + std::to_string(CurrentTheme) + ".mp3");
    musicPlayer.Play(MusicPlayer::MainTheme, true);
}

#pragma region Routines

void World::give_coin() {
    if (player.Data.Level == 3u && m_RequiredCoinsForOneUp) {
        m_SpawnOneUp = --m_RequiredCoinsForOneUp == 0;
    }

    if (++player.Data.Coins == 100u) {
        player.Data.Coins = 0u;
        player.ExtraLife();
    } else {
        audioPlayer.Play(AudioPlayer::CoinAcquire);
    }

    player.Data.Score += 200u;
}

void World::StartCutscene(std::unique_ptr<Cutscene> scene) {
    m_Cutscene = std::move(scene);
}

void World::stopCutscene() {
    m_Cutscene.reset();
}

void World::collectCoinAboveBlock(unsigned int x, unsigned int y) {
    if (
        auto& block = m_Tiles[World::GetIndex(x, y) - 1]; block &&
        Is(block.get(), Blocks::Coin)
    ) {
        give_coin();
        block.reset();
        spawnCoinAnimation(x, y + 2);
    }
}

#pragma region Timers

void World::updateTimers() {
    updateGameTime();
}

void World::initGameTime(uint16_t duration) {
    m_GameTime = duration;
    m_GameTimeUpdateTimer = GameTimerUpdateLength;
}

void World::updateGameTime() {
    if (!m_Cutscene && m_GameTime && --m_GameTimeUpdateTimer == 0u) {
        TickDownTimer();
    }
}

#pragma region Camera

void World::moveCamera() {
    constexpr uint8_t NormalThreshold = 7u;
    constexpr uint8_t AutoScrollThreshold = 5u;

    const float playerPos = player.xPosition();

    if (
        player.m_Velocity.x > 0.f && CameraPosition < playerPos - TileSize * NormalThreshold ||
        m_AutoScroll && CameraPosition < playerPos - TileSize * AutoScrollThreshold
    ) {
        CameraPosition += player.m_Velocity.x;
    }
}

void World::clampPlayerLeft() {
    const float offset = CameraPosition - player.xPosition();

    if (offset > 0.f) {
        player.Position.x += offset;

        if (player.m_State != Player::Stopping) {
            player.m_Velocity.x = 0.f;
        }
    }
}

#pragma region Sprite Spawning

bool World::AddSprite(std::unique_ptr<Sprite>& sprite) {
    Sprite* sprite_ptr = sprite.get();

    if (Is(sprite_ptr, Flag)) {
        // just load flag in special slot. Period
        m_Sprites[SpecialSpriteSlot] = std::move(sprite);

        return true;
    }

    const bool useSpecial = Is(sprite_ptr, StarFlag) || Is(sprite_ptr, JumpSpring);

    const uint8_t totalSlots = EnemySpriteSlots + useSpecial;

    for (uint8_t i = 0u; i < totalSlots; ++i) {
        auto& slot = m_Sprites[i];

        if (!slot) {
            if (Enemy* enemy = GetIf(sprite_ptr, Enemy)) {
                enemy->SlotIndex = i;

                if (LiftBalance* lift = GetIf(sprite_ptr, LiftBalance)) {
                    LiftBalance* friendLift = nullptr;

                    for (uint8_t j = 0u; j < EnemySpriteSlots; ++j) {
                        if (
                            LiftBalance* otherLift = GetIf(m_Sprites[j].get(), LiftBalance); otherLift &&
                            otherLift->m_FriendSlot == 255u && otherLift->Position.x < lift->Position.x
                        ) {
                            friendLift = otherLift;
                            break;
                        }
                    }

                    if (friendLift) {
                        lift->m_FriendSlot = friendLift->SlotIndex;
                        friendLift->m_FriendSlot = i;
                    }
                }
            }

            slot = std::move(sprite);

            return true;
        }
    }

    if (Is(sprite_ptr, StarFlag)) {
        // if no slot found, forcefully load into special slot
        m_Sprites[SpecialSpriteSlot] = std::move(sprite);

        return true;
    }

    return false;
}

void World::ReplaceSprite(std::unique_ptr<Sprite> sprite, uint8_t slotIndex) {
    if (Enemy* enemy = GetIf(sprite.get(), Enemy)) {
        enemy->SlotIndex = slotIndex;
    }

    m_Sprites[slotIndex] = std::move(sprite);
}

void World::SpawnDeathAnimation(sf::Vector2f position, uint8_t subPaletteIndex, uint8_t type, int8_t direction, float initialVelocity, uint8_t slotIndex) {
    if (m_DeathAnimations[slotIndex]) {
        for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
            if (!m_DeathAnimations[i]) {
                slotIndex = i;
                break;
            }
        }
    }

    m_DeathAnimations[slotIndex] = std::make_unique<DeathAnimation>(position, subPaletteIndex, type, direction, initialVelocity);
}

void World::handleSpriteLoading() {
    if (!m_SpritePool.empty()) {
        const float threshold = CameraPosition + (gbl::Width + TileSize * 3.f);

        std::vector<std::unique_ptr<Sprite>>& spriteGroup = m_SpritePool.front();

        if (spriteGroup.front()->Position.x <= threshold) {
            if (Is(spriteGroup.front().get(), Axe)) {
                Renderer::SetSpriteTheme(1u, 4u);
            }

            for (std::unique_ptr<Sprite>& sprite : spriteGroup) {
                if (!AddSprite(sprite)) {
                    break;
                }
            }

            m_SpritePool.erase(m_SpritePool.begin());
        }
    }
}

void World::SpawnFloateyNum(const FloateyNum& num) {
    for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
        if (!m_FloateyNums[i]) {
            SpawnFloateyNum(num, i);
            break;
        }
    }
}

void World::SpawnFloateyNum(const FloateyNum& num, uint8_t index) {
    m_FloateyNums[index] = num;
}

bool World::SpawnFireball(sf::Vector2f position, bool direction) {
    for (auto& slot : m_Fireballs) {
        if (!slot) {
            slot = std::make_unique<Fireball>(position, direction);

            return true;
        }
    }

    return false;
}

void World::SpawnFirework(sf::Vector2f position, bool type) {
    const bool index = Is(m_Fireworks[0u].get(), Firework);
    m_Fireworks[index] = std::make_unique<Firework>(position, type);
}

void World::activateJumpSpring() {
    for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
        if (auto& slot = m_Sprites[i]) {
            if (JumpSpring* spring = GetIf(slot.get(), JumpSpring)) {
                spring->Activate(*this);

                return;
            }
        }
    }
}

void World::spawnCoinAnimation(unsigned int x, unsigned int y) {
    const bool index = Is(m_BouncingCoins[0u].get(), CoinAnimation);
    m_BouncingCoins[index] = std::make_unique<CoinAnimation>(sf::Vector2f(x * TileSize, (y - 1) * TileSize - 4.f));
}

#pragma region Sprite Update

void World::updateSprites() {
    // sprite update and collision
    sf::FloatRect player_hitbox;

    const bool checkCollisions = checkEnemyCollisions();

    if (checkCollisions) {
        player_hitbox = player.getHitbox();
    }

    // move enemies
    for (auto& sprite : m_Sprites) {
        if (sprite) {
            if (Enemy* enemy = GetIf(sprite.get(), Enemy)) {
                enemy->HandleMovement(*this);
            }
        }
    }

    // update sprites
    for (auto& sprite : m_Sprites) {
        if (sprite) {
            sprite->Update(*this);

            if (checkCollisions) {
                if (Enemy* enemy = GetIf(sprite.get(), Enemy)) {
                    if (enemy->getHitbox().findIntersection(player_hitbox)) {
                        enemy->OnCollisionWithPlayer(*this);
                    } else {
                        enemy->OnNoCollision();
                    }
                }
            }
        }
    }

    if (player.isFiery()) {
        for (auto& ball : m_Fireballs) {
            if (ball) {
                ball->Update(*this);

                if (ball->ToRemove) {
                    ball.reset();
                    continue;
                }

                const sf::FloatRect ballHitbox = ball->getHitbox();

                for (auto& sprite : m_Sprites) {
                    if (
                        Enemy* enemy = GetIf(sprite.get(), Enemy); enemy &&
                        enemy->m_Type != EnemyType::DeadGoomba && enemy->m_Type != EnemyType::Lift &&
                        enemy->getHitbox().findIntersection(ballHitbox)
                    ) {
                        SpawnFirework(ball->Position, true);

                        enemy->onFireballDeath(*this, ball->getDirection());

                        ball.reset();

                        break;
                    }
                }
            }
        }
    }

    // sprite removal
    for (auto& sprite : m_Sprites) {
        if (sprite && sprite->ToRemove) {
            sprite.reset();
        }
    }

    updateHammers();

    // flip enemy collisions check as mario can only interact with enemies every other frame
    m_CheckEnemyCollisions ^= 1u;

    for (auto& animation : m_DeathAnimations) {
        if (animation) {
            animation->Update(*this);

            if (animation->ToRemove) {
                animation.reset();
            }
        }
    }

    updateBouncingBlock();

    // decrement bump timer if active
    if (m_BumpTimer) {
        --m_BumpTimer;
    }
}

bool World::SpawnHammer(const Enemy& owner) {
    constexpr uint8_t EnemySlotData[] = {4u, 4u, 4u, 5u, 5u, 5u, 6u, 6u, 6u};

    const uint8_t random = Rand::RandomInt(Rand::OffsetSpawning);
    const uint8_t hammerSlot = (random & 0b111u) ? (random & 0b111u) : (random & 0b1000u);

    if (m_Hammers[hammerSlot]) {
        return false;
    }

    const uint8_t enemySlot = EnemySlotData[hammerSlot];

    if (enemySlot <= SpecialSpriteSlot && m_Sprites[enemySlot]) {
        return false;
    }

    m_Hammers[hammerSlot] = std::make_unique<Hammer>(owner);

    return true;
}

void World::updateHammers() {
    for (uint8_t i = HammerSlots; i > 0u; --i) {
        if (auto& hammer = m_Hammers[i - 1u]) {
            hammer->Update(*this);

            if (hammer->ToRemove) {
                hammer.reset();
            }
        }
    }
}

void World::updateGrowingPowerup() {
    if (Powerup* powerup = GetIf(m_Sprites[SpecialSpriteSlot].get(), Powerup)) {
        powerup->moving_out();
    }
}

void World::updateFreezeIndependentSprites() {
    for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
        if (FloateyNum& floateyNum = m_FloateyNums[i]) {
            floateyNum.Update();
        }
    }

    for (auto& sprite : m_BouncingCoins) {
        if (sprite) {
            sprite->Update();

            if (!sprite->Active()) {
                SpawnFloateyNum(FloateyNum(sprite->Position, CameraPosition, FloateyNum::GetType(200u)));

                sprite.reset();
            }
        }
    }

    for (auto& sprite : m_Fireworks) {
        if (sprite) {
            sprite->Update();

            if (!sprite->Active()) {
                sprite.reset();
            }
        }
    }
}

#pragma region Bouncing Block

void World::handleBlockDefeat(sf::Vector2f blockPosition) {
    if (auto& sprite = m_Sprites[SpecialSpriteSlot]) {
        if (Powerup* powerup = GetIf(sprite.get(), Powerup)) {
            const sf::Vector2f powerupPosition = sf::Vector2f(powerup->xPosition(), powerup->yPosition());

            if (
                powerupPosition.y >= blockPosition.y - 16.f && powerupPosition.y <= blockPosition.y - 13.f &&
                powerupPosition.x >= blockPosition.x - 8.f && powerupPosition.x <= blockPosition.x + 8.f
            ) {
                powerup->m_Velocity = -3.f;
                powerup->Position.y -= 2.f;
                powerup->m_Direction = powerupPosition.x >= blockPosition.x ? 1 : -1;
            }
        }
    }

    for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
        if (auto& sprite = m_Sprites[i]) {
            if (Enemy* enemy = GetIf(sprite.get(), Enemy)) {
                const sf::Vector2f enemyPosition = sf::Vector2f(enemy->xPosition(), enemy->yPosition());

                if (
                    enemyPosition.y >= blockPosition.y - 32.f && enemyPosition.y <= blockPosition.y - 29.f &&
                    enemyPosition.x >= blockPosition.x - 8.f && enemyPosition.x <= blockPosition.x + 8.f
                ) {
                    enemy->onBlockDefeat(*this, blockPosition.x);

                    audioPlayer.Play(AudioPlayer::Kick);

                    uint16_t score = enemy->m_Type == EnemyType::HammerBrother ? 1000u : 100u;
                    player.Data.Score += score;
                    SpawnFloateyNum(FloateyNum(enemyPosition, CameraPosition, FloateyNum::GetType(score)), enemy->SlotIndex);

                    break;
                }
            }
        }
    }
}

void World::updateBouncingBlock() {
    if (BouncingBlock* bouncingBlock = m_BouncingBlock.get()) {
        bouncingBlock->Update(*this);

        if (m_BumpTimer == 0u) {
            if (gbl::ItemType::isPowerupItem(bouncingBlock->ItemId)) {
                spawn_powerup(bouncingBlock->ItemId, bouncingBlock->Position);
            }

            m_BouncingBlock.reset();
        }
    }
}

#pragma region Powerup

void World::spawn_powerup(uint8_t id, sf::Vector2f position) {
    if (id == gbl::PowerupType::SuperMushroom) {
        if (player.isBig()) {
            m_Sprites[SpecialSpriteSlot] = std::make_unique<FireFlower>(position);
        } else {
            m_Sprites[SpecialSpriteSlot] = std::make_unique<SuperMushroom>(position);
        }
    } else if (id == gbl::PowerupType::Starman) {
        m_Sprites[SpecialSpriteSlot] = std::make_unique<Starman>(position);
    } else if (id == gbl::PowerupType::OneUp) {
        m_Sprites[SpecialSpriteSlot] = std::make_unique<OneUp>(position);
    }
}

void World::handlePowerupCollisions() {
    if (auto& specialSlot = m_Sprites[SpecialSpriteSlot]) {
        if (
            Powerup* powerup = GetIf(specialSlot.get(), Powerup); powerup &&
            powerup->getHitbox().findIntersection(player.getHitbox())
        ) {
            powerup->GrantPower(*this);

            const uint16_t score = (powerup->m_Type != gbl::PowerupType::OneUp) * 1000u;

            if (score) {
                player.Data.Score += score;
            }

            SpawnFloateyNum(FloateyNum(sf::Vector2f(powerup->xPosition(), powerup->yPosition()), CameraPosition, FloateyNum::GetType(score)));

            specialSlot.reset();
        }
    }
}

#pragma region Player Collisions

#define ROUTINE_END_SIGNAL true
#define ROUTINE_CONTINUE_SIGNAL false

constexpr inline const uint8_t PlayerInteractionX[] = {
    8u, 3u, 12u, 2u, 13u
    // head, left foot, right foot, left side, right side
};

constexpr inline const uint8_t PlayerInteractionY[] = {
    4u, 2u, 18u, 32u, 8u, 24u
    // head big, head big in water, head small, feet, side high, side low
};

[[nodiscard]]
constexpr inline uint16_t GetFlagScore() noexcept {
    const uint8_t playerRow = static_cast<uint8_t>((player.yPosition() + 32.f) / 16.f);
    const uint8_t flagRelativePlayerRow = playerRow - 2u;

    if (flagRelativePlayerRow >= 9u) {
        return 100u;
    } else if (flagRelativePlayerRow >= 6u) {
        return 400u;
    } else if (flagRelativePlayerRow == 5u) {
        return 800u;
    } else if (flagRelativePlayerRow >= 2u) {
        return 2000u;
    } else {
        return 5000u;
    }
}

void World::collisions_PushOutOfBlockRightwards() {
    if (player.m_Velocity.x <= 0.f && (player.m_Velocity.x < 0.f || !player.m_RightKeyHeld)) {
        ++player.Position.x;
        player.on_side_collision();
    }
}

void World::collisions_PushOutOfBlockLeftwards() {
    if (player.m_Velocity.x >= 0.f && (player.m_Velocity.x > 0.f || !player.m_LeftKeyHeld)) {
        --player.Position.x;
        player.on_side_collision();
    }
}

bool World::collisions_CollisionResolveSide(float pointX, float pointY, unsigned int row, unsigned int col, std::unique_ptr<Blocks::Block>& block_ptr, bool direction) {
    // colliding with a coin
    if (collisions_CoinCheck(block_ptr, col, row)) {
        return true;
    }

    // colliding with a lift
    if (collisions_PointInLift(sf::Vector2f(pointX, pointY))) {
        if (direction == gbl::Direction::Left) {
            collisions_PushOutOfBlockRightwards();
        } else {
            collisions_PushOutOfBlockLeftwards();
        }

        return true;
    }

    const Blocks::Block* block = block_ptr.get();

    // colliding with a tile
    if (HasComponent(block, Components::Collision)) {
        // tile is visible
        if (!HasComponent(block, Components::Hidden)) {
            if (direction == gbl::Direction::Left) {
                collisions_PushOutOfBlockRightwards();
            } else {
                collisions_PushOutOfBlockLeftwards();
            }
        }

        return true;
    }

    return false;
}

void World::collisions_LandOnTile(Blocks::Block*& block) {
    const float top = player.yPosition();
    const float flooredWhole = std::truncf(top);
    const float decimalPart = top - flooredWhole;
    const float roundedWhole = std::truncf(flooredWhole / 16.f) * 16.f;

    player.Position.y = roundedWhole + decimalPart;

    m_StompChain = 0u;

    if (HasComponent(block, Blocks::JumpSpringTrigger)) {
        activateJumpSpring();
    } else {
        player.on_feet_collision();
    }
}

void World::collisions_LandOnLift(float liftTop) {
    const float top = player.yPosition();
    const float flooredWhole = std::truncf(top);
    const float decimalPart = top - flooredWhole;

    player.Position.y = liftTop - 32.f + decimalPart;

    player.on_feet_collision();

    m_StompChain = 0u;
}

void World::collisions_BonkHead() {
    const float fract = player.m_Velocity.y - std::truncf(player.m_Velocity.y);

    player.m_Velocity.y = 2.f + fract;

    player.on_head_collision();

    audioPlayer.Play(AudioPlayer::BlockHit);
}

Lift* World::collisions_PointInLift(sf::Vector2f point) {
    for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
        if (auto& sprite = m_Sprites[i]) {
            if (
                Lift* lift = GetIf(sprite.get(), Lift); lift &&
                lift->getHitbox().contains(point)
            ) {
                return lift;
            }
        }
    }

    return nullptr;
}

bool World::collisions_CoinCheck(std::unique_ptr<Blocks::Block>& block, unsigned int col, unsigned int row) {
    if (Is(block.get(), Blocks::Coin)) {
        give_coin();

        if (CurrentTheme == 0u) {
            block = std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Liquid_2);
            m_AttributeTable[World::GetIndex(col, row)] = 2u;
        } else {
            block.reset();
        }

        return true;
    }

    return false;
}

bool World::collisions_FlagCheck(Blocks::Block* block, unsigned int index) {
    if (!m_Cutscene && Is(block, Blocks::Flag)) {
        const float x = static_cast<float>(static_cast<unsigned int>(index / gbl::Rows)) * 16.f;

        if ((player.xPosition() + 16.f) - x >= 4.f) {
            StartCutscene(std::make_unique<FlagpoleScene>(*this));

            const uint16_t score = GetFlagScore();

            player.Data.Score += score;

            if (Flag* flag = GetIf(m_Sprites[SpecialSpriteSlot].get(), Flag)) {
                flag->m_FloateyNumType = FloateyNum::GetType(score);
            }

            return true;
        }
    }

    return false;
}

bool World::collisions_WarpPipeCheck(Blocks::Block* block) {
    if (!player.m_OnGround || m_Cutscene) {
        return false;
    }

    if (const Components::Render* renderComponent = GetComponent(block, const Components::Render)) {
        if (
            renderComponent->TextureId == gbl::TextureId::Block::Pipe_8 ||
            renderComponent->TextureId == gbl::TextureId::Block::Pipe_5
        ) {
            StartCutscene(std::make_unique<LPipeScene>(*this));

            m_CollisionMode = YCollision;

            m_ScrollLocked = true;

            return true;
        }
    }

    return false;
}

void World::resolvePlayerTileCollisions() {
    const float playerTop = std::truncf(player.yPosition());

    if (playerTop >= gbl::Height) {
        if (!m_Cutscene) {
            player.Kill(*this, true);
        }

        return;
    }

    if (playerTop >= 207.f || playerTop < 0.f) {
        return;
    }

    if (m_CollisionMode & YCollision) {
        if (resolvePlayerHeadCollisions(playerTop)) {
            return;
        }

        if (resolvePlayerFootCollisions(playerTop)) {
            return;
        }
    }

    if (playerTop < 16.f) {
        return;
    }

    if (m_CollisionMode & XCollision) {
        if (resolvePlayerSideCollisions(playerTop)) {
            return;
        }
    }
}

#pragma region Head

bool World::resolvePlayerHeadCollisions(float playerTop) {
    if (playerTop >= (player.isVisuallyBig() ? 32.f : 16.f)) {
        const float pointX = static_cast<float>(PlayerInteractionX[0u]) + player.xPosition();
        const float pointY = static_cast<float>(PlayerInteractionY[player.isVisuallyBig() ? (player.m_SwimmingPhysics ? 1u : 0u) : 2u]) + playerTop;

        const unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        const unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        const unsigned int index = World::GetIndex(col, row);

        auto& block_ptr = m_Tiles[index];

        // colliding with a coin
        if (collisions_CoinCheck(block_ptr, col, row)) {
            return ROUTINE_END_SIGNAL;
        }

        // colliding with a lift
        if (player.m_Velocity.y < 0.f && collisions_PointInLift(sf::Vector2f(pointX, pointY))) {
            collisions_BonkHead();

            return ROUTINE_CONTINUE_SIGNAL;
        }

        Blocks::Block* block = block_ptr.get();

        // colliding with a tile
        if (HasComponent(block, Components::Collision)) {
            // moving upwards
            if (player.m_Velocity.y < 0.f) {
                // penetration is under threshold
                if (pointY >= row * TileSize + TilePenetrationThreshold) {
                    if (m_BumpTimer || player.m_SwimmingPhysics || !HasComponent(block, Components::Hitable)) {
                        collisions_BonkHead();
                    } else {
                        // specific block interaction
                        if (player.isBig() && HasComponent(block, Components::Breakable)) {
                            // break the tile

                            player.m_Velocity.y = -2.f;
                            player.on_head_collision();

                            block_ptr.reset();
                            audioPlayer.Play(AudioPlayer::BrickSmash);

                            player.Data.Score += 50u;

                            handleBlockDefeat(sf::Vector2f(col, row + 2u) * TileSize);
                        } else {
                            // hit the block, spawn powerup or coin if present (handled by the on_block_hit_from_bottom method)

                            player.m_Velocity.y = 0.f;
                            player.on_head_collision();

                            on_block_hit_from_bottom(col, row);
                        }

                        collectCoinAboveBlock(col, row);

                        // prevent bumping for the next 16 frames
                        m_BumpTimer = 16u;
                    }

                    return ROUTINE_CONTINUE_SIGNAL;
                } else {
                    return ROUTINE_CONTINUE_SIGNAL;
                }
            } else {
                return ROUTINE_CONTINUE_SIGNAL;
            }
        }
    }

    return ROUTINE_CONTINUE_SIGNAL;
}

bool World::resolvePlayerFootCollisions(float playerTop) {
    if (playerTop >= 207.f) {
        return ROUTINE_CONTINUE_SIGNAL;
    }

    player.m_OnGround = false;

#pragma region Left Foot

    /* left foot */ {
        const float pointX = static_cast<float>(PlayerInteractionX[1u]) + player.xPosition();
        const float pointY = static_cast<float>(PlayerInteractionY[3u]) + playerTop;

        const unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        const unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        const unsigned int index = World::GetIndex(col, row);

        auto& block_ptr = m_Tiles[index];

        // touched flag
        if (collisions_FlagCheck(block_ptr.get(), index)) {
            return ROUTINE_END_SIGNAL;
        }

        // colliding with a coin
        if (collisions_CoinCheck(block_ptr, col, row)) {
            return ROUTINE_END_SIGNAL;
        }

        // colliding with a lift
        if (player.m_Velocity.y > 0.f) {
            if (Lift* lift = collisions_PointInLift(sf::Vector2f(pointX, pointY))) {
                collisions_LandOnLift(lift->yPosition());

                lift->OnPlayerLand(*this);

                return ROUTINE_CONTINUE_SIGNAL;
            }
        }

        Blocks::Block* block = block_ptr.get();

        // colliding with a tile
        if (HasComponent(block, Components::Collision)) {
            // moving downwards
            if (player.m_Velocity.y > 0.f) {
                // tile is invisible
                if (HasComponent(block, Components::Hidden)) {
                    return ROUTINE_CONTINUE_SIGNAL;
                } else {
                    // penetration is under threshold
                    if (pointY - 32.f <= (static_cast<float>(row) * TileSize + TilePenetrationThreshold)) {
                        collisions_LandOnTile(block);

                        if (player.m_DownKeyHeld && (player.xPosition() >= col * TileSize + 4.f) && HasComponent(block, Components::Warp)) {
                            StartCutscene(std::make_unique<DPipeScene>(*this));
                        }
                    } else {
                        collisions_PushOutOfBlockRightwards();
                    }

                    return ROUTINE_CONTINUE_SIGNAL;
                }
            } else {
                return ROUTINE_CONTINUE_SIGNAL;
            }
        }
    }

#pragma region Right Foot

    /* right foot */ {
        const float pointX = static_cast<float>(PlayerInteractionX[2u]) + player.xPosition();
        const float pointY = static_cast<float>(PlayerInteractionY[3u]) + playerTop;

        const unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        const unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        const unsigned int index = World::GetIndex(col, row);

        auto& block_ptr = m_Tiles[index];

        // touched flag
        if (collisions_FlagCheck(block_ptr.get(), index)) {
            return ROUTINE_END_SIGNAL;
        }

        // colliding with a coin
        if (collisions_CoinCheck(block_ptr, col, row)) {
            return ROUTINE_END_SIGNAL;
        }

        // colliding with a lift
        if (player.m_Velocity.y > 0.f) {
            if (Lift* lift = collisions_PointInLift(sf::Vector2f(pointX, pointY))) {
                collisions_LandOnLift(lift->yPosition());

                lift->OnPlayerLand(*this);

                return ROUTINE_CONTINUE_SIGNAL;
            }
        }

        Blocks::Block* block = block_ptr.get();

        // colliding with a tile
        if (HasComponent(block, Components::Collision)) {
            // moving downwards
            if (player.m_Velocity.y > 0.f) {
                // tile is invisible
                if (HasComponent(block, Components::Hidden)) {
                    return ROUTINE_CONTINUE_SIGNAL;
                } else {
                    // penetration is under threshold
                    if (pointY - 32.f <= (static_cast<float>(row) * TileSize + TilePenetrationThreshold)) {
                        collisions_LandOnTile(block);
                    } else {
                        collisions_PushOutOfBlockLeftwards();
                    }

                    return ROUTINE_CONTINUE_SIGNAL;
                }
            } else {
                return ROUTINE_CONTINUE_SIGNAL;
            }
        }
    }

    return ROUTINE_CONTINUE_SIGNAL;
}

bool World::resolvePlayerSideCollisions(float playerTop) {
    if (playerTop >= 32.f) {
    #pragma region Topleft Side

        /* topleft side */ {
            const float pointX = static_cast<float>(PlayerInteractionX[3u]) + player.xPosition();
            const float pointY = static_cast<float>(PlayerInteractionY[player.isVisuallyBig() ? 4u : 5u]) + playerTop;

            const unsigned int col = static_cast<unsigned int>(pointX / TileSize);
            const unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

            const unsigned int index = World::GetIndex(col, row);

            auto& block_ptr = m_Tiles[index];

            if (collisions_CollisionResolveSide(pointX, pointY, row, col, block_ptr, gbl::Direction::Left)) {
                return ROUTINE_END_SIGNAL;
            }
        }
    }

#pragma region Bottomleft Side

    /* bottomleft side */ {
        const float pointX = static_cast<float>(PlayerInteractionX[3u]) + player.xPosition();
        const float pointY = static_cast<float>(PlayerInteractionY[5u]) + playerTop;

        const unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        const unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        if (collisions_CollisionResolveSide(pointX, pointY, row, col, m_Tiles[World::GetIndex(col, row)], gbl::Direction::Left)) {
            return ROUTINE_END_SIGNAL;
        }
    }

    if (playerTop >= 32.f) {
    #pragma region Topright Side

        /* topright side */ {
            const float pointX = static_cast<float>(PlayerInteractionX[4u]) + player.xPosition();
            const float pointY = static_cast<float>(PlayerInteractionY[player.isVisuallyBig() ? 4u : 5u]) + playerTop;

            const unsigned int col = static_cast<unsigned int>(pointX / TileSize);
            const unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

            const unsigned int index = World::GetIndex(col, row);

            auto& block_ptr = m_Tiles[index];

            // collided with pipe
            if (collisions_WarpPipeCheck(block_ptr.get())) {
                return ROUTINE_END_SIGNAL;
            }

            if (collisions_CollisionResolveSide(pointX, pointY, row, col, block_ptr, gbl::Direction::Right)) {
                return ROUTINE_END_SIGNAL;
            }
        }
    }

#pragma region Bottomright Side

    /* bottomright side */ {
        const float pointX = static_cast<float>(PlayerInteractionX[4u]) + player.xPosition();
        const float pointY = static_cast<float>(PlayerInteractionY[5u]) + playerTop;

        const unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        const unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        const unsigned int index = World::GetIndex(col, row);

        auto& block_ptr = m_Tiles[index];

        // collided with pipe
        if (collisions_WarpPipeCheck(block_ptr.get())) {
            return ROUTINE_END_SIGNAL;
        }

        if (collisions_CollisionResolveSide(pointX, pointY, row, col, m_Tiles[World::GetIndex(col, row)], gbl::Direction::Right)) {
            return ROUTINE_END_SIGNAL;
        }
    }

    return ROUTINE_END_SIGNAL;
}
