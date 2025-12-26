#ifndef MAP_LOADER_HPP
#define MAP_LOADER_HPP

#include "World.hpp"
#include "Renderer.hpp"

class MapLoader {
private:
    struct SceneData {
        int8_t Terrain;
        int8_t Scenery;
        int8_t background;
    };

    static void placeBlock(const unsigned int& tileIndex, std::unique_ptr<Blocks::Block> block, const uint8_t& subPalleteIndex, World& world);
    static void spawnSprite(std::unique_ptr<Sprite> sprite, World& world);
    static void placeBlockIfEmpty(const unsigned int& tileIndex, std::unique_ptr<Blocks::Block> block, const uint8_t& subPalleteIndex, World& world);
    static void hiddenOneUpLogic(World& world);
    static void loadMapProperties(World& world);
    static void setTheme(const uint8_t& mapAttribute);
    static void createNewPage(World& world);
    static void loadPage(World& world);
    static void parseTileObject(const uint8_t& b1, const uint8_t& b2, World& world, const unsigned int& pageColumnOffset);
    static void parseBadGuysObject(const uint8_t& b1, const uint8_t& b2, const bool& globalDifficulty, World& world, const unsigned int& pageColumnOffset);
    static std::vector<std::pair<unsigned int, SceneData>> getBackgroundodifiers();
    static void handleCellingTerrain(const uint8_t& length, const uint8_t& block, const unsigned int& colIndex, World& world);
    static void handleMiddleTerrain(const uint8_t& length, const uint8_t& block, const unsigned int& colIndex, World& world);
    static void handleSurfaceTerrain(const uint8_t& length, const uint8_t& block_1, const uint8_t& block_2, const unsigned int& colIndex, World& world);
    static void handleInWaterBackground(const unsigned int& colIndex, World& world);
    static void handleWallBackground(const unsigned int& colIndex, World& world);
    static void handleOverWaterBackground(const unsigned int& colIndex, World& world);
    static void extendMap(World& world);

    static void onMazeTrigger(uint8_t targetPage);
    static void handleMaze(World& world);

    template <typename BlockType, typename... Args>
    static inline void placeHorizontalRow(const unsigned int& index, const uint8_t& length, const uint8_t& subPalleteIndex, World& world, Args&&... args) {
        for (uint8_t i = 0u; i < length; ++i) {
            placeBlock(index + i * 13u, std::make_unique<BlockType>(std::forward<Args>(args)...), subPalleteIndex, world);
        }
    }

    template <typename BlockType, typename... Args>
    static inline void placeVerticalRow(const unsigned int& index, const uint8_t& length, const uint8_t& subPalleteIndex, World& world, Args&&... args) {
        for (uint8_t i = 0u; i < length; ++i) {
            unsigned int tileIndex = index + i;

            auto& block = world.m_Tiles[tileIndex];
            
            if (!block || !Is(block.get(), Blocks::Coin)) {
                placeBlock(tileIndex, std::make_unique<BlockType>(std::forward<Args>(args)...), subPalleteIndex, world);
            }
        }
    }

    template <typename EnemyType>
    static inline void spawnEnemyGroup(const float& xPos, uint8_t y, const uint8_t& count, World& world) {
        const float yPos = (y + 1u) * 16.f;

        auto it = std::lower_bound(
            world.m_SpritePool.begin(), world.m_SpritePool.end(), xPos,
            [](const std::vector<std::unique_ptr<Sprite>>& group, const float& xValue) {
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
    static void placeHorizontalAir(const unsigned int& tileIndex, const uint8_t& length, World& world);
    static void placeVerticalAir(const unsigned int& tileIndex, const uint8_t& length, World& world);
    static void placeAirHole(const uint8_t& length, const unsigned int& tileIndex, World& world);
    static void placeWaterHole(const uint8_t& length, const unsigned int& tileIndex, World& world);
    static void placeHole(const uint8_t& length, const unsigned int& tileIndex, World& world);
    static void placeHoleFilledWithWater(const uint8_t& length, const unsigned int& tileIndex, World& world);
    static void placeLiftsVerticalRope(const unsigned int& colIndex, World& world);
    static void placeCastle(const uint8_t& length, const unsigned int& colIndex, const uint8_t& xPos, const unsigned int& pageColumnOffset, World& world);
    static void placeBridge(uint8_t v, const uint8_t& length, const unsigned int& colIndex, World& world);
    static void placeStaircase(const uint8_t& length, const uint8_t& metadata, const unsigned int& colIndex, World& world);
    static void placeHorizontalQuestionBlocksCoin(uint8_t v, const uint8_t& length, const unsigned int& colIndex, World& world);
    static void placeFlagpole(const unsigned int& colIndex, const uint8_t& xPos, const unsigned int& pageColumnOffset, World& world);
    static void placeSidewaysPipe(const unsigned int& tileIndex, World& world);
    static void placeJumpSpring(const unsigned int& tileIndex, const uint8_t& xPos, const unsigned int& pageColumnOffset, World& world);
    static void placeTree(const uint8_t& yPos, const uint8_t& length, const unsigned int& tileIndex, const unsigned int& colIndex, World& world);
    static void placeMushroom(const uint8_t& yPos, const uint8_t& length, const unsigned int& tileIndex, World& world);
    static void placeGun(const uint8_t& yPos, const uint8_t& length, const unsigned int& tileIndex, World& world);
    static void placeIsland(const uint8_t& yPos, const uint8_t& length, const unsigned int& tileIndex, const unsigned int& colIndex, const uint8_t& mapType, World& world);
    static void placeLongReverseLPipe(const unsigned int& colIndex, World& world);
    static void placePipe(const uint8_t& length, bool enterable, const unsigned int& tileIndex, const uint8_t& xPos, const uint8_t& yPos, const unsigned int& pageColumnOffset, World& world);

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
    static uint8_t GetIfDuplicate(const uint8_t& level, const uint8_t& stage);

    static void NewLevel(World& world, const uint8_t& areaPointer);

    static void Update(World& world);

    static const uint8_t& GetCurrentPage() {
        return CurrentPage;
    }
};

#endif // !MAP_LOADER_HPP