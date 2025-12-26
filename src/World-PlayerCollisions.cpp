#include "World.hpp"

#define ROUTINE_END_SIGNAL true
#define ROUTINE_CONTINUE_SIGNAL false

[[nodiscard]] constexpr inline uint16_t GetFlagScore() noexcept {
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

bool World::collisions_CollisionResolveSide(const float& pointX, const float& pointY, const unsigned int& row, const unsigned int& col, std::unique_ptr<Blocks::Block>& block_ptr, bool direction) {
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

void World::collisions_LandOnLift(const float& liftTop) {
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

Lift* World::collisions_PointInLift(const sf::Vector2f& point) {
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

bool World::collisions_CoinCheck(std::unique_ptr<Blocks::Block>& block, const unsigned int& col, const unsigned int& row) {
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

bool World::collisions_FlagCheck(Blocks::Block* block, const unsigned int& index) {
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

#pragma region Master

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
    if (playerTop >= (player.isVisualyBig() ? 32.f : 16.f)) {
        const float pointX = static_cast<float>(Player::InteractionX[0u]) + player.xPosition();
        const float pointY = static_cast<float>(Player::InteractionY[player.isVisualyBig() ? (player.m_SwimmingPhysics ? 1u : 0u) : 2u]) + playerTop;

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
                if (pointY >= row * TileSize + TilePenetrationThreshhold) {
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

                            handleBlockDefeat(sf::Vector2f(col, row) * TileSize);
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
        const float pointX = static_cast<float>(Player::InteractionX[1u]) + player.xPosition();
        const float pointY = static_cast<float>(Player::InteractionY[3u]) + playerTop;

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
                    if (pointY - 32.f <= (static_cast<float>(row) * TileSize + TilePenetrationThreshhold)) {
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
        const float pointX = static_cast<float>(Player::InteractionX[2u]) + player.xPosition();
        const float pointY = static_cast<float>(Player::InteractionY[3u]) + playerTop;

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
                    if (pointY - 32.f <= (static_cast<float>(row) * TileSize + TilePenetrationThreshhold)) {
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
            const float pointX = static_cast<float>(Player::InteractionX[3u]) + player.xPosition();
            const float pointY = static_cast<float>(Player::InteractionY[player.isVisualyBig() ? 4u : 5u]) + playerTop;

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
        const float pointX = static_cast<float>(Player::InteractionX[3u]) + player.xPosition();
        const float pointY = static_cast<float>(Player::InteractionY[5u]) + playerTop;

        const unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        const unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        if (collisions_CollisionResolveSide(pointX, pointY, row, col, m_Tiles[World::GetIndex(col, row)], gbl::Direction::Left)) {
            return ROUTINE_END_SIGNAL;
        }
    }

    if (playerTop >= 32.f) {
    #pragma region Topright Side

        /* topright side */ {
            const float pointX = static_cast<float>(Player::InteractionX[4u]) + player.xPosition();
            const float pointY = static_cast<float>(Player::InteractionY[player.isVisualyBig() ? 4u : 5u]) + playerTop;

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
        const float pointX = static_cast<float>(Player::InteractionX[4u]) + player.xPosition();
        const float pointY = static_cast<float>(Player::InteractionY[5u]) + playerTop;

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