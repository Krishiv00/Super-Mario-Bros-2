#ifndef GLOBALS
#define GLOBALS

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif // M_PI

[[nodiscard]] inline constexpr float toRad(float degrees) {
    return degrees * M_PI / 180.f;
}

#define DEBUG

#ifdef DEBUG
    #include <iostream>
#endif // DEBUG

#include <cmath>

#include "Utils/Rand.hpp"

namespace gbl {
    constexpr uint8_t Fps = 60u;

    constexpr unsigned int Width = 256u;
    constexpr unsigned int Height = 240u;

    constexpr uint8_t Rows = 15u;

    template <typename _T>
    constexpr inline int8_t sign(_T x) {
        return x >= _T(0) ? 1 : -1;
    }

    enum Direction : bool {
        Left, Right
    };

    namespace TextureId {
        enum Block : uint8_t {
            None,
            GrassPlatform_1,
            GrassPlatform_2,
            GrassPlatform_3,
            MushroomPlatform_1,
            MushroomPlatform_2,
            MushroomPlatform_3,
            GrassAndCloudTop_1,
            GrassAndCloudTop_2,
            GrassAndCloudTop_3,
            Grass_hill_1,
            Grass_hill_2,
            Grass_hill_3,
            Grass_hill_4,
            Grass_hill_5,
            Grass_hill_6,
            Bush,
            SeaWeed,
            Pipe_1,
            Pipe_2,
            Pipe_3,
            Pipe_4,
            Pipe_5,
            Pipe_6,
            Pipe_7,
            Pipe_8,
            Pipe_9,
            Pipe_10,
            Flag_top,
            Flag_pole,
            Bush_tall_1,
            Bush_tall_2,
            Chain,
            Bridge_top,
            Brick_1,
            Brick_2,
            Brick_3,
            Floor_1,
            Floor_2,
            Flag_base,
            TrampolineBase,
            Castle_1,
            Castle_2,
            Castle_3,
            Castle_4,
            Castle_5,
            Castle_6,
            GrassPlatform_base,
            MushroomPlatform_middle,
            MushroomPlatform_base,
            Fence,
            Plant_bottom,
            Bridge_base,
            Anchor_1,
            String_1,
            Anchor_2,
            String_2,
            BulletBill_1,
            BulletBill_2,
            BulletBill_3,
            CloudBottom_1,
            CloudBottom_2,
            CloudBottom_3,
            Cloud_platform,
            Liquid_1,
            Liquid_2,
            Bridge_bowser,
            Question,
            Question_used,
            Coin
        };
    }

    namespace PowerupType {
        enum Type : uint8_t {
            SuperMushroom,
            Starman,
            OneUp,
            FireFlower
        };
    }

    namespace ItemType {
        enum Type : uint8_t {
            SuperMushroom = PowerupType::SuperMushroom,
            FireFlower    = PowerupType::FireFlower,
            Starman       = PowerupType::Starman,
            OneUp         = PowerupType::OneUp,
            Coin,
            Vine,
            None
        };

        constexpr inline bool isCoinItem(uint8_t itemType) {
            return itemType == Coin;
        }
        
        constexpr inline bool isPowerupItem(uint8_t itemType) {
            return itemType != None && !isCoinItem(itemType);
        }
    }
}

#ifdef DEBUG
    #include "Utils/Stopwatch.hpp"
    
    #define LOG_ERROR(x) std::cerr << __FILE__ << "-\"" << x << "\"" << std::endl
    #define PRINT(x) std::cout << (x) << std::endl
#else
    #define LOG_ERROR(x)
    #define PRINT(x)
#endif // DEBUG

constexpr float TileSize = 16.f;

#endif // !GLOBALS