#include "World.hpp"
#include "Renderer.hpp"

#pragma region Sprite Spawning

bool World::AddSprite(std::unique_ptr<Sprite>& sprite) {
    Sprite* sprite_ptr = sprite.get();

    if (GetIf(sprite_ptr, Flag)) {
        // just load flag in special slot. Period
        m_Sprites[SpecialSpriteSlot] = std::move(sprite);

        return true;
    }

    const bool useSpecial = GetIf(sprite_ptr, StarFlag) || GetIf(sprite_ptr, JumpSpring);

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
                            LiftBalance* otherLift = GetIf(m_Sprites[j].get(), LiftBalance);
                            otherLift && otherLift->m_FriendSlot == 255u && otherLift->Position.x < lift->Position.x
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

    if (GetIf(sprite_ptr, StarFlag)) {
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

void World::SpawnDeathAnimation(sf::Vector2f position, uint8_t subPalleteIndex, uint8_t type, int8_t direction, float initialVelocity, uint8_t slotIndex) {
    auto& slot = m_DeathAnimations[slotIndex];

    if (slot) {
        for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
            if (!m_DeathAnimations[i]) {
                slotIndex = std::move(i);
                break;
            }
        }
    }

    slot = std::make_unique<DeathAnimation>(position, subPalleteIndex, type, direction, initialVelocity);
}

void World::handleSpriteLoading() {
    if (!m_SpritePool.empty()) {
        const float threshold = CameraPosition + gbl::Width + TileSize * 3.f;

        std::vector<std::unique_ptr<Sprite>>& spriteGroup = m_SpritePool[0u];

        if (spriteGroup.front()->Position.x <= threshold) {
            if (GetIf(spriteGroup.front().get(), Axe)) {
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
            SpawnFloateyNum(std::move(num), i);
            break;
        }
    }
}

void World::SpawnFloateyNum(const FloateyNum& num, uint8_t index) {
    m_FloateyNums[index] = std::move(num);
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
    m_MiscSprites[1u] = std::make_unique<Firework>(position, type);
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
    bool index = GetIf(m_MiscSprites[0u].get(), CoinAnimation);
    m_MiscSprites[index] = std::make_unique<CoinAnimation>(sf::Vector2f(x * TileSize, (y - 1) * TileSize - 4.f));
}

#pragma region Sprite Update

void World::updateSprites() {
    // sprite update and collision
    sf::FloatRect player_hitbox = player.getHitbox();

    bool checkCollisions = m_CheckEnemyCollisions && !player.IsFrozen();

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

                sf::FloatRect ballHitbox = ball->getHitbox();

                for (auto& sprite : m_Sprites) {
                    if (
                        Enemy* enemy = GetIf(sprite.get(), Enemy);
                        enemy && enemy->m_Type != EnemyType::DeadGoomba && !GetIf(sprite.get(), Lift) &&
                        enemy->getHitbox().findIntersection(ballHitbox)
                    ) {
                        SpawnFirework(ball->Position, true);
                        
                        enemy->onFireballDeath(*this, ball->m_Direction);
                        
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

void World::updateGrowingPowerup() {
    if (Powerup* powerup = GetComponent(m_Sprites[SpecialSpriteSlot].get(), Powerup)) {
        powerup->moving_out();
    }
}

void World::updateFreezeIndependentSprites() {
    for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
        if (FloateyNum& floateyNum = m_FloateyNums[i]) {
            floateyNum.Update();
        }
    }

    for (auto& sprite : m_MiscSprites) {
        if (sprite) {
            sprite->Update();

            if (!sprite->Active()) {
                if (GetIf(sprite.get(), CoinAnimation)) {
                    SpawnFloateyNum(FloateyNum(sprite->Position, CameraPosition, FloateyNum::GetType(200u)));
                }

                sprite.reset();
            }
        }
    }
}

#pragma region Bouncing Block

void World::handleBlockDefeat(sf::Vector2f blockPosition) {
    if (auto& sprite = m_Sprites[SpecialSpriteSlot]) {
        if (Powerup* powerup = GetIf(sprite.get(), Powerup)) {
            sf::Vector2f powerupPosition = sf::Vector2f(powerup->xPosition(), powerup->yPosition());

            if (
                powerupPosition.y >= blockPosition.y - 16.f && powerupPosition.y <= blockPosition.y - 13.f &&
                powerupPosition.x >= blockPosition.x - 8.f && powerupPosition.x <= blockPosition.x + 8.f
            ) {
                powerup->m_Velocity = -3.f;
                powerup->Position.y -= 2.f;
                powerup->m_Direction = powerupPosition.x > blockPosition.x ? 1 : -1;
            }
        }
    }

    for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
        if (auto& sprite = m_Sprites[i]) {
            if (Enemy* enemy = GetIf(sprite.get(), Enemy)) {
                sf::Vector2f enemyPosition = sf::Vector2f(enemy->xPosition(), enemy->yPosition());

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
            Powerup* powerup = GetIf(specialSlot.get(), Powerup);
            powerup &&
            powerup->getHitbox().findIntersection(player.getHitbox())
        ) {
            powerup->GrantPower(*this);

            uint16_t score = powerup->m_Type == gbl::PowerupType::OneUp ? 0u : 1000u;

            if (score) {
                player.Data.Score += score;
            }

            SpawnFloateyNum(FloateyNum(sf::Vector2f(powerup->xPosition(), powerup->yPosition()), CameraPosition, FloateyNum::GetType(score)));

            specialSlot.reset();
        }
    }
}