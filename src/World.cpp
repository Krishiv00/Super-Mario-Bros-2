#include "World.hpp"

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

    for (FloateyNum& floateyNum : m_FloateyNums) {
        floateyNum.Reset();
    }

    for (auto& sprite : m_MiscSprites) {
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

bool World::PointInTile(const sf::Vector2f& point) const {
    const unsigned int col = static_cast<unsigned int>(point.x / TileSize);
    const unsigned int row = static_cast<unsigned int>(point.y / TileSize) - 2;

    const auto& block = m_Tiles[World::GetIndex(col, row)];

    return block && GetComponent(block.get(), Components::Collision);
}

#pragma region Events

void World::on_block_hit_from_bottom(unsigned int x, unsigned int y) {
    updateBouncingBlock();

    auto& block = m_Tiles[World::GetIndex(x, y)];

    if (Components::Item* itemComponent = GetComponent(block.get(), Components::Item)) {
        bool use = true;

        if (gbl::ItemType::isPowerupItem(itemComponent->ItemId)) {
            m_Sprites[SpecialSpriteSlot].reset();

            audioPlayer.Play(AudioPlayer::PowerupSpawn);
        } else if (gbl::ItemType::isCoinItem(itemComponent->ItemId)) {
            if (
                Components::Render* renderComponent = GetComponent(block.get(), Components::Render);
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

        m_BouncingBlock = std::make_unique<BouncingBlock>(block, itemComponent->ItemId, m_BumpTimer, x, y, m_AttributeTable[World::GetIndex(x, y)]);

        handleBlockDefeat(m_BouncingBlock->Position);
    } else if (GetComponent(block.get(), Components::Hitable)) {
        m_BouncingBlock = std::make_unique<BouncingBlock>(block, gbl::ItemType::None, m_BumpTimer, x, y, m_AttributeTable[World::GetIndex(x, y)]);

        handleBlockDefeat(m_BouncingBlock->Position);
    }

    audioPlayer.Play(AudioPlayer::BlockHit);
}

void World::on_enter_warp_zone() {
    m_SpawnOneUp = true;
}

void World::on_otaining_supermushroom() {
    StartCutscene(std::make_unique<GrowingScene>(*this));
}

void World::on_otaining_fireflower() {
    StartCutscene(std::make_unique<FireFlowerScene>(*this));
}

void World::on_otaining_starman() {
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
        auto& block = m_Tiles[World::GetIndex(x, y) - 1];
        block &&
        GetIf(block.get(), Blocks::Coin)
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
    if (player.m_Velocity.x > 0.f && CameraPosition < player.xPosition() - gbl::Width * 0.4f - 8.f) {
        CameraPosition += player.m_Velocity.x;
    } else if (getAutoScroll()) {
        if (CameraPosition - player.xPosition() < TileSize * 3.f + gbl::Width * 0.5f) {
            ++CameraPosition;
        }
    }
}

void World::clampPlayerLeft() {
    float offset = CameraPosition - player.xPosition();

    if (offset > 0.f) {
        player.Position.x += offset;

        if (player.m_State != Player::Stopping) {
            player.m_Velocity.x = 0.f;
        }
    }
}