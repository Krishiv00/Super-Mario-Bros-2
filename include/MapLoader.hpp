#pragma once

#include "World.hpp"
#include "Renderer.hpp"

class MapLoader {
private:
    struct SceneData {
        int8_t Terrain;
        int8_t Scenery;
        int8_t background;
    };

    static void placeBlock(unsigned int tileIndex, std::unique_ptr<Blocks::Block> block, uint8_t subPalleteIndex, World& world);
    static void spawnSprite(std::unique_ptr<Sprite> sprite, World& world);
    static void placeBlockIfEmpty(unsigned int tileIndex, std::unique_ptr<Blocks::Block> block, uint8_t subPalleteIndex, World& world);
    static void hiddenOneUpLogic(World& world);
    static void loadMapProperties(World& world);
    static void setTheme(uint8_t mapAttribute);
    static void createNewPage(World& world);
    static void loadPage(World& world);
    static void parseTileObject(uint8_t b1, uint8_t b2, World& world, unsigned int pageColumnOffset);
    static void parseBadGuysObject(uint8_t b1, uint8_t b2, bool globalDifficulty, World& world, unsigned int pageColumnOffset);
    static std::vector<std::pair<unsigned int, SceneData>> getBackgroundodifiers();
    static void handleCellingTerrain(uint8_t length, uint8_t block, unsigned int colIndex, World& world);
    static void handleMiddleTerrain(uint8_t length, uint8_t block, unsigned int colIndex, World& world);
    static void handleSurfaceTerrain(uint8_t length, uint8_t block_1, uint8_t block_2, unsigned int colIndex, World& world);
    static void handleInWaterBackground(unsigned int colIndex, World& world);
    static void handleWallBackground(unsigned int colIndex, World& world);
    static void handleOverWaterBackground(unsigned int colIndex, World& world);
    static void extendMap(World& world);

    static void onMazeTrigger(uint8_t targetPage);
    static void handleMaze(World& world);

    template <typename BlockType, typename... Args>
    static inline void placeHorizontalRow(unsigned int index, uint8_t length, uint8_t subPalleteIndex, World& world, Args&&... args) {
        for (uint8_t i = 0u; i < length; ++i) {
            placeBlock(index + i * 13u, std::make_unique<BlockType>(std::forward<Args>(args)...), subPalleteIndex, world);
        }
    }

    template <typename BlockType, typename... Args>
    static inline void placeVerticalRow(unsigned int index, uint8_t length, uint8_t subPalleteIndex, World& world, Args&&... args) {
        for (uint8_t i = 0u; i < length; ++i) {
            unsigned int tileIndex = index + i;

            auto& block = world.m_Tiles[tileIndex];
            
            if (!block || !Is(block.get(), Blocks::Coin)) {
                placeBlock(tileIndex, std::make_unique<BlockType>(std::forward<Args>(args)...), subPalleteIndex, world);
            }
        }
    }

    template <typename EnemyType>
    static inline void spawnEnemyGroup(float xPos, uint8_t y, uint8_t count, World& world) {
        const float yPos = (y + 1u) * 16.f;

        auto it = std::lower_bound(
            world.m_SpritePool.begin(), world.m_SpritePool.end(), xPos,
            [](const std::vector<std::unique_ptr<Sprite>>& group, float xValue) {
            return group.front()->Position.x < xValue;
        }
        );

        std::vector<std::unique_ptr<Sprite>>& newGroup = *world.m_SpritePool.insert(it, std::vector<std::unique_ptr<Sprite>>{});

        newGroup.reserve(count);
        for (uint8_t i = 0; i < count; ++i) {
            newGroup.push_back(std::make_unique<EnemyType>(sf::Vector2f(xPos + i * 24.f, yPos)));
        }
    }

    // Tile Structures
    static void placeHorizontalAir(unsigned int tileIndex, uint8_t length, World& world);
    static void placeVerticalAir(unsigned int tileIndex, uint8_t length, World& world);
    static void placeAirHole(uint8_t length, unsigned int tileIndex, World& world);
    static void placeWaterHole(uint8_t length, unsigned int tileIndex, World& world);
    static void placeHole(uint8_t length, unsigned int tileIndex, World& world);
    static void placeHoleFilledWithWater(uint8_t length, unsigned int tileIndex, World& world);
    static void placeLiftsVerticalRope(unsigned int colIndex, World& world);
    static void placeCastle(uint8_t length, unsigned int colIndex, uint8_t xPos, unsigned int pageColumnOffset, World& world);
    static void placeBridge(uint8_t v, uint8_t length, unsigned int colIndex, World& world);
    static void placeStaircase(uint8_t length, uint8_t metadata, unsigned int colIndex, World& world);
    static void placeHorizontalQuestionBlocksCoin(uint8_t v, uint8_t length, unsigned int colIndex, World& world);
    static void placeFlagpole(unsigned int colIndex, uint8_t xPos, unsigned int pageColumnOffset, World& world);
    static void placeSidewaysPipe(unsigned int tileIndex, World& world);
    static void placeJumpSpring(unsigned int tileIndex, uint8_t xPos, unsigned int pageColumnOffset, World& world);
    static void placeTree(uint8_t yPos, uint8_t length, unsigned int tileIndex, unsigned int colIndex, World& world);
    static void placeMushroom(uint8_t yPos, uint8_t length, unsigned int tileIndex, World& world);
    static void placeGun(uint8_t yPos, uint8_t length, unsigned int tileIndex, World& world);
    static void placeIsland(uint8_t yPos, uint8_t length, unsigned int tileIndex, unsigned int colIndex, uint8_t mapType, World& world);
    static void placeLongReverseLPipe(unsigned int colIndex, World& world);
    static void placePipe(uint8_t length, bool enterable, unsigned int tileIndex, uint8_t xPos, uint8_t yPos, unsigned int pageColumnOffset, World& world);

    // Map Properties
    static inline uint8_t CurrentTerrainType;
    static inline uint8_t CurrentSceneryType;
    static inline uint8_t CurrentBackgroundType;
    static inline uint8_t CurrentPage;
    static inline uint8_t MapType;

    static inline const uint8_t* TileData;
    static inline uint8_t TileDataIterator;
    static inline uint8_t TileSkipToPage;
    static inline bool LastTileObjectPageFlag;

    static inline const uint8_t* BadGuysData;
    static inline uint8_t BadGuysDataIterator;
    static inline uint8_t BadGuysSkipToPage;
    static inline bool LastBadGuysObjectPageFlag;
    static inline bool LastBadGuysObjectWasPageSkip;
    static inline bool LastBadGuysObjectWasBalanceLift;

    static inline uint8_t CurrentMazeScore;

    static inline uint8_t HoleBufferLength;
    static inline std::vector<std::pair<unsigned int, SceneData>> BackgroundModifiers;

public:
    static uint8_t GetIfDuplicate(uint8_t level, uint8_t stage);

    static void NewLevel(World& world, uint8_t areaPointer);

    static void Update(World& world);

    static uint8_t GetCurrentPage() {
        return CurrentPage;
    }
};