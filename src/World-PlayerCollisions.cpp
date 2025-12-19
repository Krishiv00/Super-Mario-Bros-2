#include "World.hpp"

void World::collisions_PushOutOfBlockRightwards() {
    const float& velocity = player.m_Velocity.x;

    if (velocity <= 0.f && (velocity < 0.f || !player.m_RightKeyHeld)) {
        ++player.Position.x;
        player.on_side_collision();
    }
}

void World::collisions_PushOutOfBlockLeftwards() {
    const float& velocity = player.m_Velocity.x;

    if (velocity >= 0.f && (velocity > 0.f || !player.m_LeftKeyHeld)) {
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

    Blocks::Block* block = block_ptr.get();

    // colliding with a tile
    if (GetComponent(block, Components::Collision)) {
        // tile is visible
        if (!GetComponent(block, Components::Hidden)) {
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

    if (GetComponent(block, Blocks::JumpSpringTrigger)) {
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
    float fract = player.m_Velocity.y - std::truncf(player.m_Velocity.y);

    player.m_Velocity.y = 2.f + fract;

    player.on_head_collision();

    audioPlayer.Play(AudioPlayer::BlockHit);
}

Lift* World::collisions_PointInLift(const sf::Vector2f& point) {
    for (uint8_t i = 0u; i < EnemySpriteSlots; ++i) {
        auto& sprite = m_Sprites[i];

        if (sprite) {
            if (
                Lift* lift = GetIf(sprite.get(), Lift);
                lift && lift->getHitbox().contains(point)
            ) {
                return lift;
            }
        }
    }

    return nullptr;
}

bool World::collisions_CoinCheck(std::unique_ptr<Blocks::Block>& block, const unsigned int& col, const unsigned int& row) {
    if (GetIf(block.get(), Blocks::Coin)) {
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
    if (!m_Cutscene && GetIf(block, Blocks::Flag)) {
        float x = static_cast<float>(static_cast<unsigned int>(index / gbl::Rows)) * 16.f;

        if ((player.xPosition() + 16.f) - x >= 4.f) {
            StartCutscene(std::make_unique<FlagpoleScene>(*this));

            uint8_t playerRow = static_cast<uint8_t>((player.yPosition() + 32.f) / 16.f);
            uint8_t flagRelativePlayerRow = playerRow - 2u;

            uint16_t score;

            if (flagRelativePlayerRow >= 9u) {
                score = 100u;
            } else if (flagRelativePlayerRow >= 6u) {
                score = 400u;
            } else if (flagRelativePlayerRow == 5u) {
                score = 800u;
            } else if (flagRelativePlayerRow >= 2u) {
                score = 2000u;
            } else {
                score = 5000u;
            }

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

    if (Components::Render* renderComponent = GetComponent(block, Components::Render)) {
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
    float playerTop = std::truncf(player.yPosition());

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
#define ROUTINE_END_SIGNAL true
#define ROUTINE_CONTINUE_SIGNAL false

    if (playerTop >= (player.isVisualyBig() ? 32.f : 16.f)) {
        float pointX = static_cast<float>(Player::InteractionX[0u]) + player.xPosition();
        float pointY = static_cast<float>(Player::InteractionY[player.isVisualyBig() ? (player.m_SwimmingPhysics ? 1u : 0u) : 2u]) + playerTop;

        unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        unsigned int index = World::GetIndex(col, row);

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
        if (GetComponent(block, Components::Collision)) {
            // moving upwards
            if (player.m_Velocity.y < 0.f) {
                // penetration is under threshold
                if (pointY >= row * TileSize + TilePenetrationThreshhold) {
                    if (m_BumpTimer || player.m_SwimmingPhysics || !GetComponent(block, Components::Hitable)) {
                        collisions_BonkHead();
                    } else {
                        // specific block interaction
                        if (player.isBig() && GetComponent(block, Components::Breakable)) {
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
#define ROUTINE_END_SIGNAL true
#define ROUTINE_CONTINUE_SIGNAL false

    if (playerTop >= 207.f) {
        return ROUTINE_CONTINUE_SIGNAL;
    }

    player.m_OnGround = false;

#pragma region Left Foot

    /* left foot */ {
        float pointX = static_cast<float>(Player::InteractionX[1u]) + player.xPosition();
        float pointY = static_cast<float>(Player::InteractionY[3u]) + playerTop;

        unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        unsigned int index = World::GetIndex(col, row);

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
        if (GetComponent(block, Components::Collision)) {
            // moving downwards
            if (player.m_Velocity.y > 0.f) {
                // tile is invisible
                if (GetComponent(block, Components::Hidden)) {
                    return ROUTINE_CONTINUE_SIGNAL;
                } else {
                    // penetration is under threshold
                    if (pointY - 32.f <= (static_cast<float>(row) * TileSize + TilePenetrationThreshhold)) {
                        collisions_LandOnTile(block);

                        if (player.m_DownKeyHeld && (player.xPosition() >= col * TileSize + 4.f) && GetComponent(block, Components::Warp)) {
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
        float pointX = static_cast<float>(Player::InteractionX[2u]) + player.xPosition();
        float pointY = static_cast<float>(Player::InteractionY[3u]) + playerTop;

        unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        unsigned int index = World::GetIndex(col, row);

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
        if (GetComponent(block, Components::Collision)) {
            // moving downwards
            if (player.m_Velocity.y > 0.f) {
                // tile is invisible
                if (GetComponent(block, Components::Hidden)) {
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
#define ROUTINE_END_SIGNAL true
#define ROUTINE_CONTINUE_SIGNAL false

    if (playerTop >= 32.f) {
    #pragma region Topleft Side

        /* topleft side */ {
            float pointX = static_cast<float>(Player::InteractionX[3u]) + player.xPosition();
            float pointY = static_cast<float>(Player::InteractionY[player.isVisualyBig() ? 4u : 5u]) + playerTop;

            unsigned int col = static_cast<unsigned int>(pointX / TileSize);
            unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

            unsigned int index = World::GetIndex(col, row);

            auto& block_ptr = m_Tiles[index];

            // touched flag
            if (collisions_FlagCheck(block_ptr.get(), index)) {
                return ROUTINE_END_SIGNAL;
            }

            if (collisions_CollisionResolveSide(pointX, pointY, row, col, block_ptr, gbl::Direction::Left)) {
                return ROUTINE_END_SIGNAL;
            }
        }
    }

#pragma region Bottomleft Side

    /* bottomleft side */ {
        float pointX = static_cast<float>(Player::InteractionX[3u]) + player.xPosition();
        float pointY = static_cast<float>(Player::InteractionY[5u]) + playerTop;

        unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        if (collisions_CollisionResolveSide(pointX, pointY, row, col, m_Tiles[World::GetIndex(col, row)], gbl::Direction::Left)) {
            return ROUTINE_END_SIGNAL;
        }
    }

    if (playerTop >= 32.f) {
    #pragma region Topright Side

        /* topright side */ {
            float pointX = static_cast<float>(Player::InteractionX[4u]) + player.xPosition();
            float pointY = static_cast<float>(Player::InteractionY[player.isVisualyBig() ? 4u : 5u]) + playerTop;

            unsigned int col = static_cast<unsigned int>(pointX / TileSize);
            unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

            unsigned int index = World::GetIndex(col, row);

            auto& block_ptr = m_Tiles[index];

            // touched flag
            if (collisions_FlagCheck(block_ptr.get(), index)) {
                return ROUTINE_END_SIGNAL;
            }

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
        float pointX = static_cast<float>(Player::InteractionX[4u]) + player.xPosition();
        float pointY = static_cast<float>(Player::InteractionY[5u]) + playerTop;

        unsigned int col = static_cast<unsigned int>(pointX / TileSize);
        unsigned int row = static_cast<unsigned int>(pointY / TileSize) - 2;

        unsigned int index = World::GetIndex(col, row);

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