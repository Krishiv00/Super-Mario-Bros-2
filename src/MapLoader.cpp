#include "MapLoader.hpp"

constexpr inline uint8_t ObjectXPos(const uint8_t& b1, const uint8_t&) {
    return (b1 >> 0x04u) & 0x0Fu;
}

constexpr inline uint8_t ObjectYPos(const uint8_t& b1, const uint8_t&) {
    return b1 & 0x0Fu;
}

constexpr inline uint8_t EnemyYPos(const uint8_t& b1, const uint8_t&) {
    return b1 & 0x0Fu;
}

constexpr inline uint8_t ObjectCategory(const uint8_t&, const uint8_t& b2) {
    return (b2 >> 0x04u) & 0x07u;
}

constexpr inline uint8_t EnemyCategory(const uint8_t&, const uint8_t& b2) {
    return b2 & 0x3F;
}

constexpr inline bool ObjectPageFlag(const uint8_t&, const uint8_t& b2) {
    return b2 & 0x80u;
}

constexpr inline uint8_t ObjectMetadata(const uint8_t&, const uint8_t& b2) {
    return b2 & 0x0Fu;
}

constexpr inline uint8_t ObjectLength(const uint8_t& b1, const uint8_t& b2) {
    const uint8_t category = ObjectCategory(b1, b2);
    const uint8_t metadata = ObjectMetadata(b1, b2);

    if (category == 0x00u || category == 0x01u || category == 0x04u || category == 0x05u || category == 0x06u) {
        return (metadata & 0x0Fu) + 0x01u;
    }

    else if (category == 0x2u) {
        if (ObjectYPos(b1, b2) == 0x0Fu) {
            return metadata;
        }

        return metadata + 0x01u;
    }

    else if (category == 0x03u) {
        return metadata + 0x01u;
    }

    else if (category == 0x07u) {
        return (metadata & 0x07u) + 0x01u;
    }

    return 0x01u;
}

constexpr inline uint8_t ObjectIsPageSkip(const uint8_t& b1, const uint8_t& b2) {
    if (ObjectYPos(b1, b2) == 0x0Du && ObjectCategory(b1, b2) != 0x04u) {
        return b2;
    } else {
        return 0x00u;
    }
}

constexpr inline uint8_t getbackgroundTile(const uint8_t& x, const uint8_t& y, const uint8_t& page, const uint8_t& scenery) {
    constexpr uint8_t BackgroundSets[][3u][16u * 11u] = {
        {
            {
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 7u, 8u, 8u, 9u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 60, 61, 61, 62, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 7u, 8u, 9u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 60, 61, 62, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                9u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                62, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u
            },

            {
                0u, 0u, 7u, 8u, 8u, 9u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 60, 61, 61, 62, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 7u, 8u, 9u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 60, 61, 62, 0u
            },

            {
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 7u, 8u, 9u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 7u, 8u, 9u, 60, 61, 62, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 60, 61, 62, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 7u, 8u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 60, 61
            }
        },

        {
            {
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 7u, 8u, 9u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 60, 61, 62, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 10, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 11, 12, 15, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                11, 12, 13, 14, 15, 0u, 0u, 0u, 0u, 0u, 0u, 7u, 8u, 8u, 8u, 9u
            },

            {
                0u, 0u, 0u, 7u, 8u, 9u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 60, 61, 62, 0u, 0u, 0u, 0u, 0u, 7u, 8u, 8u, 8u, 9u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 60, 61, 61, 61, 62,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 10, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                11, 12, 15, 0u, 0u, 0u, 0u, 7u, 8u, 9u, 0u, 0u, 0u, 0u, 0u, 0u
            },

            {
                0u, 0u, 0u, 0u, 7u, 8u, 8u, 9u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 60, 61, 61, 62, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 7u, 8u, 8u, 9u, 0u, 0u, 0u
            }
        },

        {
            {
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                7u, 8u, 8u, 9u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                60, 61, 61, 62, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 30, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 16, 0u, 31, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 51, 0u, 51, 50, 50
            },

            {
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 7u, 8u, 9u, 0u, 0u,
                0u, 0u, 7u, 8u, 9u, 0u, 0u, 0u, 0u, 0u, 0u, 60, 61, 62, 7u, 8u,
                0u, 0u, 60, 61, 62, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 60, 61,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 30, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 31, 0u, 16, 16, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                50, 50, 0u, 0u, 0u, 51, 0u, 51, 51, 0u, 0u, 0u, 0u, 0u, 0u, 0u
            },

            {
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 7u, 8u, 9u,
                8u, 9u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 60, 61, 62,
                61, 62, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 30, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 16, 0u, 0u, 31, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u, 0u, 50, 50, 51, 50, 0u, 51, 0u, 0u, 0u, 0u
            }
        }
    };
    return BackgroundSets[scenery - 1][page % 3u][y * 16u + x];
}

constexpr uint8_t GetSubPalleteIndex(uint8_t blockId, uint8_t row = 255u, uint8_t mapTheme = 255u) {
    constexpr uint8_t GrassThreshold = 10u;

    if (
        blockId == gbl::TextureId::Block::GrassPlatform_1 ||
        blockId == gbl::TextureId::Block::GrassPlatform_2 ||
        blockId == gbl::TextureId::Block::GrassPlatform_3 ||
        blockId == gbl::TextureId::Block::MushroomPlatform_1 ||
        blockId == gbl::TextureId::Block::MushroomPlatform_2 ||
        blockId == gbl::TextureId::Block::MushroomPlatform_3 ||
        blockId == gbl::TextureId::Block::SeaWeed ||
        blockId == gbl::TextureId::Block::Pipe_1 ||
        blockId == gbl::TextureId::Block::Pipe_2 ||
        blockId == gbl::TextureId::Block::Pipe_3 ||
        blockId == gbl::TextureId::Block::Pipe_4 ||
        blockId == gbl::TextureId::Block::Pipe_6 ||
        blockId == gbl::TextureId::Block::Pipe_7 ||
        blockId == gbl::TextureId::Block::Pipe_9 ||
        blockId == gbl::TextureId::Block::Pipe_10 ||
        blockId == gbl::TextureId::Block::Grass_hill_1 ||
        blockId == gbl::TextureId::Block::Grass_hill_2 ||
        blockId == gbl::TextureId::Block::Grass_hill_3 ||
        blockId == gbl::TextureId::Block::Grass_hill_4 ||
        blockId == gbl::TextureId::Block::Grass_hill_5 ||
        blockId == gbl::TextureId::Block::Grass_hill_6 ||
        blockId == gbl::TextureId::Block::Bush ||
        blockId == gbl::TextureId::Block::Bush_tall_1 ||
        blockId == gbl::TextureId::Block::Bush_tall_2 ||
        blockId == gbl::TextureId::Block::Chain ||
        blockId == gbl::TextureId::Block::Bridge_top ||
        blockId == gbl::TextureId::Block::Flag_top ||
        blockId == gbl::TextureId::Block::Flag_pole
    ) {
        return 0u;
    }

    else if (
        blockId == gbl::TextureId::Block::Pipe_5 ||
        blockId == gbl::TextureId::Block::Pipe_8
    ) {
        return mapTheme == 0u;
    }

    else if (
        blockId == gbl::TextureId::Block::BulletBill_1 ||
        blockId == gbl::TextureId::Block::BulletBill_2 ||
        blockId == gbl::TextureId::Block::BulletBill_3 ||
        blockId == gbl::TextureId::Block::Brick_1 ||
        blockId == gbl::TextureId::Block::Brick_2 ||
        blockId == gbl::TextureId::Block::Brick_3 ||
        blockId == gbl::TextureId::Block::Floor_1 ||
        blockId == gbl::TextureId::Block::Floor_2 ||
        blockId == gbl::TextureId::Block::Flag_base ||
        blockId == gbl::TextureId::Block::Bridge_base ||
        blockId == gbl::TextureId::Block::TrampolineBase ||
        blockId == gbl::TextureId::Block::Castle_1 ||
        blockId == gbl::TextureId::Block::Castle_2 ||
        blockId == gbl::TextureId::Block::Castle_3 ||
        blockId == gbl::TextureId::Block::Castle_4 ||
        blockId == gbl::TextureId::Block::Castle_5 ||
        blockId == gbl::TextureId::Block::Castle_6 ||
        blockId == gbl::TextureId::Block::GrassPlatform_base ||
        blockId == gbl::TextureId::Block::MushroomPlatform_middle ||
        blockId == gbl::TextureId::Block::MushroomPlatform_base ||
        blockId == gbl::TextureId::Block::Fence ||
        blockId == gbl::TextureId::Block::Plant_bottom ||
        blockId == gbl::TextureId::Block::Anchor_1 ||
        blockId == gbl::TextureId::Block::Anchor_2 ||
        blockId == gbl::TextureId::Block::String_1 ||
        blockId == gbl::TextureId::Block::String_2
    ) {
        return 1u;
    }

    else if (
        blockId == gbl::TextureId::Block::GrassAndCloudTop_1 ||
        blockId == gbl::TextureId::Block::GrassAndCloudTop_2 ||
        blockId == gbl::TextureId::Block::GrassAndCloudTop_3
    ) {
        return row == GrassThreshold ? 0u : 2u;
    }

    else if (
        blockId == gbl::TextureId::Block::CloudBottom_1 ||
        blockId == gbl::TextureId::Block::CloudBottom_2 ||
        blockId == gbl::TextureId::Block::CloudBottom_3 ||
        blockId == gbl::TextureId::Block::Liquid_1 ||
        blockId == gbl::TextureId::Block::Liquid_2 ||
        blockId == gbl::TextureId::Block::Cloud_platform ||
        blockId == gbl::TextureId::Block::Bridge_bowser
    ) {
        return 2u;
    }

    else if (
        blockId == gbl::TextureId::Block::Question ||
        blockId == gbl::TextureId::Block::Question_used ||
        blockId == gbl::TextureId::Block::Coin
    ) {
        return 3u;
    }

    return 0u;
}

constexpr inline void getTerrainData(uint8_t& cellingLength, uint8_t& middleLength, uint8_t& surfaceLength, uint8_t& terrainBlock_1, uint8_t& terrainBlock_2, const uint8_t& mapAttribute, const uint8_t& terrainType) {
    if (mapAttribute == 0x00u) {
        terrainBlock_1 = gbl::TextureId::Block::Floor_2;
        terrainBlock_2 = gbl::TextureId::Block::Floor_2;
    }

    else if (mapAttribute == 0x01u) {
        terrainBlock_1 = gbl::TextureId::Block::Floor_1;
        terrainBlock_2 = gbl::TextureId::Block::Floor_1;
    }

    else if (mapAttribute == 0x02u) {
        terrainBlock_1 = gbl::TextureId::Block::Brick_2;
        terrainBlock_2 = gbl::TextureId::Block::Floor_1;
    }

    else {
        terrainBlock_1 = gbl::TextureId::Block::Brick_3;
        terrainBlock_2 = gbl::TextureId::Block::Brick_3;
    }

    if (terrainType == 0x01u) {
        surfaceLength = 0x02u;
    }

    else if (terrainType == 0x02u) {
        cellingLength = 0x01u;
        surfaceLength = 0x02u;
    }

    else if (terrainType == 0x03u) {
        cellingLength = 0x03u;
        surfaceLength = 0x02u;
    }

    else if (terrainType == 0x04u) {
        cellingLength = 0x04u;
        surfaceLength = 0x02u;
    }

    else if (terrainType == 0x05u) {
        cellingLength = 0x08u;
        surfaceLength = 0x02u;
    }

    else if (terrainType == 0x06u) {
        cellingLength = 0x01u;
        surfaceLength = 0x05u;
    }

    else if (terrainType == 0x07u) {
        cellingLength = 0x03u;
        surfaceLength = 0x05u;
    }

    else if (terrainType == 0x08u) {
        cellingLength = 0x04u;
        surfaceLength = 0x05u;
    }

    else if (terrainType == 0x09u) {
        cellingLength = 0x01u;
        surfaceLength = 0x06u;
    }

    else if (terrainType == 0x0Au) {
        cellingLength = 0x01u;
        surfaceLength = 0x00u;
    }

    else if (terrainType == 0x0Bu) {
        cellingLength = 0x04u;
        surfaceLength = 0x06u;
    }

    else if (terrainType == 0x0Cu) {
        cellingLength = 0x01u;
        surfaceLength = 0x09u;
    }

    else if (terrainType == 0x0Du) {
        cellingLength = 0x01u;
        middleLength = 0x05u;
        surfaceLength = 0x02u;
    }

    else if (terrainType == 0x0Eu) {
        cellingLength = 0x01u;
        middleLength = 0x04u;
        surfaceLength = 0x02u;
    }

    else if (terrainType == 0x0Fu) {
        // Brick:All
        cellingLength = 0x0Bu;
        surfaceLength = 0x02u;
    }
}

uint8_t MapLoader::GetIfDuplicate(const uint8_t& level, const uint8_t& stage) {
    if (level == 0x05u && stage == 0x03u) {
        return 0x01u;
    }

    if (level == 0x05u && stage == 0x04u) {
        return 0x02u;
    }

    if (level == 0x06u && stage == 0x04u) {
        return 0x01u;
    }

    if (level == 0x07u && (stage == 0x02u || stage == 0x03u)) {
        return 0x02u;
    }

    return level;
}

constexpr inline uint8_t getBasicBrick(const uint8_t& mapAttribute) {
    if (mapAttribute == 0x00u) {
        return gbl::TextureId::Block::SeaWeed;
    }

    else if (mapAttribute == 0x01u) {
        return gbl::TextureId::Block::Brick_1;
    }

    return gbl::TextureId::Block::Brick_2;
}

constexpr inline uint8_t getBasicBlock(const uint8_t& mapAttribute) {
    if (mapAttribute == 0x00u) {
        return gbl::TextureId::Block::Floor_2;
    }

    if (mapAttribute == 0x03u) {
        return gbl::TextureId::Block::Brick_3;
    }

    return gbl::TextureId::Block::Flag_base;
}

std::vector<std::pair<unsigned int, MapLoader::SceneData>> MapLoader::getBackgroundodifiers() {
    std::vector<std::pair<unsigned int, SceneData>> backgroundModifiers;

    uint8_t page = CurrentPage;
    uint8_t i = TileDataIterator;

    while (true) {
        const uint8_t& b1 = TileData[i];

        if (b1 == 0xFDu) {
            break; // end of tile structure data
        }

        const uint8_t& b2 = TileData[(++i)++];

        if (ObjectPageFlag(b1, b2)) {
            ++page;
        } else if (const uint8_t pageSkips = ObjectIsPageSkip(b1, b2)) {
            page = pageSkips;
            continue;
        }

        if (ObjectYPos(b1, b2) == 0x0Eu) {
            // Change Background
            unsigned int colIndex = (page * 16u + ObjectXPos(b1, b2)) * 13u;
            uint8_t metadata = ObjectMetadata(b1, b2);

            if ((b2 >> 0x06u) & 0x01u) {
                // Change background
                uint8_t newBackground = metadata;

                backgroundModifiers.push_back(std::make_pair(colIndex, SceneData(-1, -1, newBackground)));
            } else {
                // Terrain and Scenery
                uint8_t newTerrain = metadata;
                uint8_t newScenery = (b2 >> 0x04u) & 0x03u;

                backgroundModifiers.push_back(std::make_pair(colIndex, SceneData(newTerrain, newScenery, -1)));
            }
        }
    }

    return backgroundModifiers;
}

const uint8_t* GetTileData(const uint8_t& areaPointer) {
    static std::vector<uint8_t> Data[] = {
        // 1-1
        std::vector<uint8_t> {
        // header
        0x50, 0x21, 0x15,
            // tile data
            0x07, 0x81, 0x47, 0x24, 0x57, 0x00, 0x63, 0x01, 0x77, 0x01,
            0xC9, 0x71, 0x68, 0xF2, 0xE7, 0x73, 0x97, 0xFB, 0x06, 0x83,
            0x5C, 0x01, 0xD7, 0x22, 0xE7, 0x00, 0x03, 0xA7, 0x6C, 0x02,
            0xB3, 0x22, 0xE3, 0x01, 0xE7, 0x07, 0x47, 0xA0, 0x57, 0x06,
            0xA7, 0x01, 0xD3, 0x00, 0xD7, 0x01, 0x07, 0x81, 0x67, 0x20,
            0x93, 0x22, 0x03, 0xA3, 0x1C, 0x61, 0x17, 0x21, 0x6F, 0x33,
            0xC7, 0x63, 0xD8, 0x62, 0xE9, 0x61, 0xFA, 0x60, 0x4F, 0xB3,
            0x87, 0x63, 0x9C, 0x01, 0xB7, 0x63, 0xC8, 0x62, 0xD9, 0x61,
            0xEA, 0x60, 0x39, 0xF1, 0x87, 0x21, 0xA7, 0x01, 0xB7, 0x20,
            0x39, 0xF1, 0x5F, 0x38, 0x6D, 0xC1, 0xAF, 0x26,
            0xFDu
    },

        // 1-2
        std::vector<uint8_t> {
        // header
        0x48, 0x0F, 0x26,
            // tile data
            0x0E, 0x01, 0x5E, 0x02, 0xA7, 0x00, 0xBC, 0x73, 0x1A, 0xE0,
            0x39, 0x61, 0x58, 0x62, 0x77, 0x63, 0x97, 0x63, 0xB8, 0x62,
            0xD6, 0x07, 0xF8, 0x62, 0x19, 0xE1, 0x75, 0x52, 0x86, 0x40,
            0x87, 0x50, 0x95, 0x52, 0x93, 0x43, 0xA5, 0x21, 0xC5, 0x52,
            0xD6, 0x40, 0xD7, 0x20, 0xE5, 0x06, 0xE6, 0x51, 0x3E, 0x8D,
            0x5E, 0x03, 0x67, 0x52, 0x77, 0x52, 0x7E, 0x02, 0x9E, 0x03,
            0xA6, 0x43, 0xA7, 0x23, 0xDE, 0x05, 0xFE, 0x02, 0x1E, 0x83,
            0x33, 0x54, 0x46, 0x40, 0x47, 0x21, 0x56, 0x04, 0x5E, 0x02,
            0x83, 0x54, 0x93, 0x52, 0x96, 0x07, 0x97, 0x50, 0xBE, 0x03,
            0xC7, 0x23, 0xFE, 0x02, 0x0C, 0x82, 0x43, 0x45, 0x45, 0x24,
            0x46, 0x24, 0x90, 0x08, 0x95, 0x51, 0x78, 0xFA, 0xD7, 0x73,
            0x39, 0xF1, 0x8C, 0x01, 0xA8, 0x52, 0xB8, 0x52, 0xCC, 0x01,
            0x5F, 0xB3, 0x97, 0x63, 0x9E, 0x00, 0x0E, 0x81, 0x16, 0x24,
            0x66, 0x04, 0x8E, 0x00, 0xFE, 0x01, 0x08, 0xD2, 0x0E, 0x06,
            0x6F, 0x47, 0x9E, 0x0F, 0x0E, 0x82, 0x2D, 0x47, 0x28, 0x7A,
            0x68, 0x7A, 0xA8, 0x7A, 0xAE, 0x01, 0xDE, 0x0F, 0x6D, 0xC5,
            0xFDu
    },

        // 1-3
        std::vector<uint8_t> {
        // header
        0x90, 0x11, 0x14,
            // tile data
            0x0F, 0x26, 0xFE, 0x10, 0x2A, 0x93, 0x87, 0x17, 0xA3, 0x14,
            0xB2, 0x42, 0x0A, 0x92, 0x19, 0x40, 0x36, 0x14, 0x50, 0x41,
            0x82, 0x16, 0x2B, 0x93, 0x24, 0x41, 0xBB, 0x14, 0xB8, 0x00,
            0xC2, 0x43, 0xC3, 0x13, 0x1B, 0x94, 0x67, 0x12, 0xC4, 0x15,
            0x53, 0xC1, 0xD2, 0x41, 0x12, 0xC1, 0x29, 0x13, 0x85, 0x17,
            0x1B, 0x92, 0x1A, 0x42, 0x47, 0x13, 0x83, 0x41, 0xA7, 0x13,
            0x0E, 0x91, 0xA7, 0x63, 0xB7, 0x63, 0xC5, 0x65, 0xD5, 0x65,
            0xDD, 0x4A, 0xE3, 0x67, 0xF3, 0x67, 0x8D, 0xC1, 0xAE, 0x42,
            0xDF, 0x20,
            0xFDu
    },

        // 1-4
        std::vector<uint8_t> {
        // header
        0x9B, 0x07, 0x30,
            // tile data
            0x05, 0x32, 0x06, 0x33, 0x07, 0x34, 0xCE, 0x03, 0xDC, 0x51,
            0xEE, 0x07, 0x73, 0xE0, 0x74, 0x0A, 0x7E, 0x06, 0x9E, 0x0A,
            0xCE, 0x06, 0xE4, 0x00, 0xE8, 0x0A, 0xFE, 0x0A, 0x2E, 0x89,
            0x4E, 0x0B, 0x54, 0x0A, 0x14, 0x8A, 0xC4, 0x0A, 0x34, 0x8A,
            0x7E, 0x06, 0xC7, 0x0A, 0x01, 0xE0, 0x02, 0x0A, 0x47, 0x0A,
            0x81, 0x60, 0x82, 0x0A, 0xC7, 0x0A, 0x0E, 0x87, 0x7E, 0x02,
            0xA7, 0x02, 0xB3, 0x02, 0xD7, 0x02, 0xE3, 0x02, 0x07, 0x82,
            0x13, 0x02, 0x3E, 0x06, 0x7E, 0x02, 0xAE, 0x07, 0xFE, 0x0A,
            0x0D, 0xC4, 0xCD, 0x43, 0xCE, 0x09, 0xDE, 0x0B, 0xDD, 0x42,
            0xFE, 0x02, 0x5D, 0xC7,
            0xFDu
    },

        // 2-1
        std::vector<uint8_t> {
        // header
        0x52, 0x31, 0x16,
            // tile data
            0x0F, 0x20, 0x6E, 0x40, 0xF7, 0x20, 0x07, 0x84, 0x17, 0x20,
            0x4F, 0x34, 0xC3, 0x03, 0xC7, 0x02, 0xD3, 0x22, 0x27, 0xE3,
            0x39, 0x61, 0xE7, 0x73, 0x5C, 0xE4, 0x57, 0x00, 0x6C, 0x73,
            0x47, 0xA0, 0x53, 0x06, 0x63, 0x22, 0xA7, 0x73, 0xFC, 0x73,
            0x13, 0xA1, 0x33, 0x05, 0x43, 0x21, 0x5C, 0x72, 0xC3, 0x23,
            0xCC, 0x03, 0x77, 0xFB, 0xAC, 0x02, 0x39, 0xF1, 0xA7, 0x73,
            0xD3, 0x04, 0xE8, 0x72, 0xE3, 0x22, 0x26, 0xF4, 0xBC, 0x02,
            0x8C, 0x81, 0xA8, 0x62, 0x17, 0x87, 0x43, 0x24, 0xA7, 0x01,
            0xC3, 0x04, 0x08, 0xF2, 0x97, 0x21, 0xA3, 0x02, 0xC9, 0x0B,
            0xE1, 0x69, 0xF1, 0x69, 0x8D, 0xC1, 0xCF, 0x26,
            0xFDu
    },

        // 2-2
        std::vector<uint8_t> {
        // header
        0x41, 0x01, 0x05,
            // tile data
            0xB8, 0x52, 0xEA, 0x41, 0x27, 0xB2, 0xB3, 0x42, 0x16, 0xD4,
            0x4A, 0x42, 0xA5, 0x51, 0xA7, 0x31, 0x27, 0xD3, 0x08, 0xE2,
            0x16, 0x64, 0x2C, 0x04, 0x38, 0x42, 0x76, 0x64, 0x88, 0x62,
            0xDE, 0x07, 0xFE, 0x01, 0x0D, 0xC9, 0x23, 0x32, 0x31, 0x51,
            0x98, 0x52, 0x0D, 0xC9, 0x59, 0x42, 0x63, 0x53, 0x67, 0x31,
            0x14, 0xC2, 0x36, 0x31, 0x87, 0x53, 0x17, 0xE3, 0x29, 0x61,
            0x30, 0x62, 0x3C, 0x08, 0x42, 0x37, 0x59, 0x40, 0x6A, 0x42,
            0x99, 0x40, 0xC9, 0x61, 0xD7, 0x63, 0x39, 0xD1, 0x58, 0x52,
            0xC3, 0x67, 0xD3, 0x31, 0xDC, 0x06, 0xF7, 0x42, 0xFA, 0x42,
            0x23, 0xB1, 0x43, 0x67, 0xC3, 0x34, 0xC7, 0x34, 0xD1, 0x51,
            0x43, 0xB3, 0x47, 0x33, 0x9A, 0x30, 0xA9, 0x61, 0xB8, 0x62,
            0xBE, 0x0B, 0xD5, 0x09, 0xDE, 0x0F, 0x0D, 0xCA, 0x7D, 0x47,
            0xFDu
    },

        // 2-3
        std::vector<uint8_t> {
        // header
        0x90, 0x11, 0x17,
            // tile data
            0x0F, 0x26, 0x6E, 0x10, 0x8B, 0x17, 0xAF, 0x32, 0xD8, 0x62,
            0xE8, 0x62, 0xFC, 0x3F, 0xAD, 0xC8, 0xF8, 0x64, 0x0C, 0xBE,
            0x43, 0x43, 0xF8, 0x64, 0x0C, 0xBF, 0x73, 0x40, 0x84, 0x40,
            0x93, 0x40, 0xA4, 0x40, 0xB3, 0x40, 0xF8, 0x64, 0x48, 0xE4,
            0x5C, 0x39, 0x83, 0x40, 0x92, 0x41, 0xB3, 0x40, 0xF8, 0x64,
            0x48, 0xE4, 0x5C, 0x39, 0xF8, 0x64, 0x13, 0xC2, 0x37, 0x65,
            0x4C, 0x24, 0x63, 0x00, 0x97, 0x65, 0xC3, 0x42, 0x0B, 0x97,
            0xAC, 0x32, 0xF8, 0x64, 0x0C, 0xBE, 0x53, 0x45, 0x9D, 0x48,
            0xF8, 0x64, 0x2A, 0xE2, 0x3C, 0x47, 0x56, 0x43, 0xBA, 0x62,
            0xF8, 0x64, 0x0C, 0xB7, 0x88, 0x64, 0xBC, 0x31, 0xD4, 0x45,
            0xFC, 0x31, 0x3C, 0xB1, 0x78, 0x64, 0x8C, 0x38, 0x0B, 0x9C,
            0x1A, 0x33, 0x18, 0x61, 0x28, 0x61, 0x39, 0x60, 0x5D, 0x4A,
            0xEE, 0x11, 0x0F, 0xB8, 0x1D, 0xC1, 0x3E, 0x42, 0x6F, 0x20,
            0xFDu
    },

        // 2-4
        std::vector<uint8_t> {
        // header
        0x9B, 0x7u, 0x30,
            // tile data
            0x05, 0x32, 0x06, 0x33, 0x07, 0x34, 0xFE, 0x00, 0x27, 0xB1,
            0x65, 0x32, 0x75, 0x0A, 0x71, 0x00, 0xB7, 0x31, 0x08, 0xE4,
            0x18, 0x64, 0x1E, 0x04, 0x57, 0x3B, 0xBB, 0x0A, 0x17, 0x8A,
            0x27, 0x3A, 0x73, 0x0A, 0x7B, 0x0A, 0xD7, 0x0A, 0xE7, 0x3A,
            0x3B, 0x8A, 0x97, 0x0A, 0xFE, 0x08, 0x24, 0x8A, 0x2E, 0x00,
            0x3E, 0x40, 0x38, 0x64, 0x6F, 0x00, 0x9F, 0x00, 0xBE, 0x43,
            0xC8, 0x0A, 0xC9, 0x63, 0xCE, 0x07, 0xFE, 0x07, 0x2E, 0x81,
            0x66, 0x42, 0x6A, 0x42, 0x79, 0x0A, 0xBE, 0x00, 0xC8, 0x64,
            0xF8, 0x64, 0x08, 0xE4, 0x2E, 0x07, 0x7E, 0x03, 0x9E, 0x07,
            0xBE, 0x03, 0xDE, 0x07, 0xFE, 0x0A, 0x03, 0xA5, 0x0D, 0x44,
            0xCD, 0x43, 0xCE, 0x09, 0xDD, 0x42, 0xDE, 0x0B, 0xFE, 0x02,
            0x5D, 0xC7,
            0xFDu
    },

        // 3-1
        std::vector<uint8_t> {
        // header
        0x52, 0x31, 0x16,
            // tile data
            0x0F, 0x20, 0x6E, 0x66, 0x07, 0x81, 0x36, 0x01, 0x66, 0x00,
            0xA7, 0x22, 0x08, 0xF2, 0x67, 0x7B, 0xDC, 0x02, 0x98, 0xF2,
            0xD7, 0x20, 0x39, 0xF1, 0x9F, 0x33, 0xDC, 0x27, 0xDC, 0x57,
            0x23, 0x83, 0x57, 0x63, 0x6C, 0x51, 0x87, 0x63, 0x99, 0x61,
            0xA3, 0x06, 0xB3, 0x21, 0x77, 0xF3, 0xF3, 0x21, 0xF7, 0x2A,
            0x13, 0x81, 0x23, 0x22, 0x53, 0x00, 0x63, 0x22, 0xE9, 0x0B,
            0x0C, 0x83, 0x13, 0x21, 0x16, 0x22, 0x33, 0x05, 0x8F, 0x35,
            0xEC, 0x01, 0x63, 0xA0, 0x67, 0x20, 0x73, 0x01, 0x77, 0x01,
            0x83, 0x20, 0x87, 0x20, 0xB3, 0x20, 0xB7, 0x20, 0xC3, 0x01,
            0xC7, 0x00, 0xD3, 0x20, 0xD7, 0x20, 0x67, 0xA0, 0x77, 0x07,
            0x87, 0x22, 0xE8, 0x62, 0xF5, 0x65, 0x1C, 0x82, 0x7F, 0x38,
            0x8D, 0xC1, 0xCF, 0x26,
            0xFDu
    },

        // 3-2
        std::vector<uint8_t> {
        // header
        0x96, 0x31, 0x16,
            // tile data
            0x0F, 0x26, 0x0D, 0x03, 0x1A, 0x60, 0x77, 0x42, 0xC4, 0x00,
            0xC8, 0x62, 0xB9, 0xE1, 0xD3, 0x06, 0xD7, 0x07, 0xF9, 0x61,
            0x0C, 0x81, 0x4E, 0xB1, 0x8E, 0xB1, 0xBC, 0x01, 0xE4, 0x50,
            0xE9, 0x61, 0x0C, 0x81, 0x0D, 0x0A, 0x84, 0x43, 0x98, 0x72,
            0x0D, 0x0C, 0x0F, 0x38, 0x1D, 0xC1, 0x5F, 0x26,
            0xFDu
    },

        // 3-3
        std::vector<uint8_t> {
        // header
        0x94, 0x11, 0x14,
            // tile data
            0x0F, 0x26, 0xFE, 0x10, 0x28, 0x94, 0x65, 0x15, 0xEB, 0x12,
            0xFA, 0x41, 0x4A, 0x96, 0x54, 0x40, 0xA4, 0x42, 0xB7, 0x13,
            0xE9, 0x19, 0xF5, 0x15, 0x11, 0x80, 0x47, 0x42, 0x71, 0x13,
            0x80, 0x41, 0x15, 0x92, 0x1B, 0x1F, 0x24, 0x40, 0x55, 0x12,
            0x64, 0x40, 0x95, 0x12, 0xA4, 0x40, 0xD2, 0x12, 0xE1, 0x40,
            0x13, 0xC0, 0x2C, 0x17, 0x2F, 0x12, 0x49, 0x13, 0x83, 0x40,
            0x9F, 0x14, 0xA3, 0x40, 0x17, 0x92, 0x83, 0x13, 0x92, 0x41,
            0xB9, 0x14, 0xC5, 0x12, 0xC8, 0x40, 0xD4, 0x40, 0x4B, 0x92,
            0x78, 0x1B, 0x9C, 0x94, 0x9F, 0x11, 0xDF, 0x14, 0xFE, 0x11,
            0x7D, 0xC1, 0x9E, 0x42, 0xCF, 0x20,
            0xFDu
    },

        // 3-4
        std::vector<uint8_t> {
        // header
        0x9B, 0x07, 0x30,
            // tile data
            0x05, 0x32, 0x06, 0x33, 0x07, 0x34, 0xFE, 0x06, 0x0C, 0x81,
            0x39, 0x0A, 0x5C, 0x01, 0x89, 0x0A, 0xAC, 0x01, 0xD9, 0x0A,
            0xFC, 0x01, 0x2E, 0x83, 0xA7, 0x01, 0xB7, 0x00, 0xC7, 0x01,
            0xDE, 0x0A, 0xFE, 0x02, 0x4E, 0x83, 0x5A, 0x32, 0x63, 0x0A,
            0x69, 0x0A, 0x7E, 0x02, 0xEE, 0x03, 0xFA, 0x32, 0x03, 0x8A,
            0x09, 0x0A, 0x1E, 0x02, 0xEE, 0x03, 0xFA, 0x32, 0x03, 0x8A,
            0x09, 0x0A, 0x14, 0x42, 0x1E, 0x02, 0x7E, 0x0A, 0x9E, 0x07,
            0xFE, 0x0A, 0x2E, 0x86, 0x5E, 0x0A, 0x8E, 0x06, 0xBE, 0x0A,
            0xEE, 0x07, 0x3E, 0x83, 0x5E, 0x07, 0xFE, 0x0A, 0x0D, 0xC4,
            0x41, 0x52, 0x51, 0x52, 0xCD, 0x43, 0xCE, 0x09, 0xDE, 0x0B,
            0xDD, 0x42, 0xFE, 0x02, 0x5D, 0xC7,
            0xFDu
    },

        // 4-1
        std::vector<uint8_t> {
        // header
        0x52, 0x21, 0x16,
            // tile data
            0x0F, 0x20, 0x6E, 0x40, 0x58, 0xF2, 0x93, 0x01, 0x97, 0x00,
            0x0C, 0x81, 0x97, 0x40, 0xA6, 0x41, 0xC7, 0x40, 0x0D, 0x04,
            0x03, 0x01, 0x07, 0x01, 0x23, 0x01, 0x27, 0x01, 0xEC, 0x03,
            0xAC, 0xF3, 0xC3, 0x03, 0x78, 0xE2, 0x94, 0x43, 0x47, 0xF3,
            0x74, 0x43, 0x47, 0xFB, 0x74, 0x43, 0x2C, 0xF1, 0x4C, 0x63,
            0x47, 0x00, 0x57, 0x21, 0x5C, 0x01, 0x7C, 0x72, 0x39, 0xF1,
            0xEC, 0x02, 0x4C, 0x81, 0xD8, 0x62, 0xEC, 0x01, 0x0D, 0x0D,
            0x0F, 0x38, 0xC7, 0x07, 0xED, 0x4A, 0x1D, 0xC1, 0x5F, 0x26,
            0xFDu
    },

        // 4-2
        std::vector<uint8_t> {
        // header
        0x48, 0x0F, 0x26,
            // tile data
            0x0E, 0x01, 0x5E, 0x02, 0xBC, 0x01, 0xFC, 0x01, 0x2C, 0x82,
            0x41, 0x52, 0x4E, 0x04, 0x67, 0x25, 0x68, 0x24, 0x69, 0x24,
            0xBA, 0x42, 0xC7, 0x04, 0xDE, 0x0B, 0xB2, 0x87, 0xFE, 0x02,
            0x2C, 0xE1, 0x2C, 0x71, 0x67, 0x01, 0x77, 0x00, 0x87, 0x01,
            0x8E, 0x00, 0xEE, 0x01, 0xF6, 0x02, 0x03, 0x85, 0x05, 0x02,
            0x13, 0x21, 0x16, 0x02, 0x27, 0x02, 0x2E, 0x02, 0x88, 0x72,
            0xC7, 0x20, 0xD7, 0x07, 0xE4, 0x76, 0x07, 0xA0, 0x17, 0x06,
            0x48, 0x7A, 0x76, 0x20, 0x98, 0x72, 0x79, 0xE1, 0x88, 0x62,
            0x9C, 0x01, 0xB7, 0x73, 0xDC, 0x01, 0xF8, 0x62, 0xFE, 0x01,
            0x08, 0xE2, 0x0E, 0x00, 0x6E, 0x02, 0x73, 0x20, 0x77, 0x23,
            0x83, 0x04, 0x93, 0x20, 0xAE, 0x00, 0xFE, 0x0A, 0x0E, 0x82,
            0x39, 0x71, 0xA8, 0x72, 0xE7, 0x73, 0x0C, 0x81, 0x8F, 0x32,
            0xAE, 0x00, 0xFE, 0x04, 0x04, 0xD1, 0x17, 0x04, 0x26, 0x49,
            0x27, 0x29, 0xDF, 0x33, 0xFE, 0x02, 0x44, 0xF6, 0x7C, 0x01,
            0x8E, 0x06, 0xBF, 0x47, 0xEE, 0x0F, 0x4D, 0xC7, 0x0E, 0x82,
            0x68, 0x7A, 0xAE, 0x01, 0xDE, 0x0F, 0x6D, 0xC5,
            0xFDu
    },

        // 4-3
        std::vector<uint8_t> {
        // header
        0x90, 0x51, 0x14,
            // tile data
            0x0F, 0x26, 0xEE, 0x10, 0x0B, 0x94, 0x33, 0x14, 0x42, 0x42,
            0x77, 0x16, 0x86, 0x44, 0x02, 0x92, 0x4A, 0x16, 0x69, 0x42,
            0x73, 0x14, 0xB0, 0x00, 0xC7, 0x12, 0x05, 0xC0, 0x1C, 0x17,
            0x1F, 0x11, 0x36, 0x12, 0x8F, 0x14, 0x91, 0x40, 0x1B, 0x94,
            0x35, 0x12, 0x34, 0x42, 0x60, 0x42, 0x61, 0x12, 0x87, 0x12,
            0x96, 0x40, 0xA3, 0x14, 0x1C, 0x98, 0x1F, 0x11, 0x47, 0x12,
            0x9F, 0x15, 0xCC, 0x15, 0xCF, 0x11, 0x05, 0xC0, 0x1F, 0x15,
            0x39, 0x12, 0x7C, 0x16, 0x7F, 0x11, 0x82, 0x40, 0x98, 0x12,
            0xDF, 0x15, 0x16, 0xC4, 0x17, 0x14, 0x54, 0x12, 0x9B, 0x16,
            0x28, 0x94, 0xCE, 0x01, 0x3D, 0xC1, 0x5E, 0x42, 0x8F, 0x20,
            0xFDu
    },

        // 4-4
        std::vector<uint8_t> {
        // header
        0x5B, 0x07, 0x30,
            // tile data
            0x05, 0x32, 0x06, 0x33, 0x07, 0x34, 0x5E, 0x0A, 0x68, 0x64,
            0x98, 0x64, 0xA8, 0x64, 0xCE, 0x06, 0xFE, 0x02, 0x0D, 0x01,
            0x1E, 0x0E, 0x7E, 0x02, 0x94, 0x63, 0xB4, 0x63, 0xD4, 0x63,
            0xF4, 0x63, 0x14, 0xE3, 0x2E, 0x0E, 0x5E, 0x02, 0x64, 0x35,
            0x88, 0x72, 0xBE, 0x0E, 0x0D, 0x04, 0xAE, 0x02, 0xCE, 0x08,
            0xCD, 0x4B, 0xFE, 0x02, 0x0D, 0x05, 0x68, 0x31, 0x7E, 0x0A,
            0x96, 0x31, 0xA9, 0x63, 0xA8, 0x33, 0xD5, 0x30, 0xEE, 0x02,
            0xE6, 0x62, 0xF4, 0x61, 0x04, 0xB1, 0x08, 0x3F, 0x44, 0x33,
            0x94, 0x63, 0xA4, 0x31, 0xE4, 0x31, 0x04, 0xBF, 0x08, 0x3F,
            0x04, 0xBF, 0x08, 0x3F, 0xCD, 0x4B, 0x03, 0xE4, 0x0E, 0x03,
            0x2E, 0x01, 0x7E, 0x06, 0xBE, 0x02, 0xDE, 0x06, 0xFE, 0x0A,
            0x0D, 0xC4, 0xCD, 0x43, 0xCE, 0x09, 0xDE, 0x0B, 0xDD, 0x42,
            0xFE, 0x02, 0x5D, 0xC7,
            0xFDu
    },

        // 5-1
        std::vector<uint8_t> {
        // header
        0x95, 0xB1, 0x16,
            // tile data
            0x0F, 0x26, 0x0D, 0x02, 0xC8, 0x72, 0x1C, 0x81, 0x38, 0x72,
            0x0D, 0x05, 0x97, 0x34, 0x98, 0x62, 0xA3, 0x20, 0xB3, 0x06,
            0xC3, 0x20, 0xCC, 0x03, 0xF9, 0x91, 0x2C, 0x81, 0x48, 0x62,
            0x0D, 0x09, 0x37, 0x63, 0x47, 0x03, 0x57, 0x21, 0x8C, 0x02,
            0xC5, 0x79, 0xC7, 0x31, 0xF9, 0x11, 0x39, 0xF1, 0xA9, 0x11,
            0x6F, 0xB4, 0xD3, 0x65, 0xE3, 0x65, 0x7D, 0xC1, 0xBF, 0x26,
            0xFDu
    },

        // 5-2
        std::vector<uint8_t> {
        // header
        0x55, 0xB1, 0x16,
            // tile data
            0x0F, 0x26, 0xCF, 0x33, 0x07, 0xB2, 0x15, 0x11, 0x52, 0x42,
            0x99, 0x0B, 0xAC, 0x02, 0xD3, 0x24, 0xD6, 0x42, 0xD7, 0x25,
            0x23, 0x84, 0xCF, 0x33, 0x07, 0xE3, 0x19, 0x61, 0x78, 0x7A,
            0xEF, 0x33, 0x2C, 0x81, 0x46, 0x64, 0x55, 0x65, 0x65, 0x65,
            0xEC, 0x74, 0x47, 0x82, 0x53, 0x05, 0x63, 0x21, 0x62, 0x41,
            0x96, 0x22, 0x9A, 0x41, 0xCC, 0x03, 0xB9, 0x91, 0x39, 0xF1,
            0x63, 0x26, 0x67, 0x27, 0xD3, 0x06, 0xFC, 0x01, 0x18, 0xE2,
            0xD9, 0x07, 0xE9, 0x04, 0x0C, 0x86, 0x37, 0x22, 0x93, 0x24,
            0x87, 0x84, 0xAC, 0x02, 0xC2, 0x41, 0xC3, 0x23, 0xD9, 0x71,
            0xFC, 0x01, 0x7F, 0xB1, 0x9C, 0x00, 0xA7, 0x63, 0xB6, 0x64,
            0xCC, 0x00, 0xD4, 0x66, 0xE3, 0x67, 0xF3, 0x67, 0x8D, 0xC1,
            0xCF, 0x26,
            0xFDu
    },

        // 5-3
        std::vector<uint8_t> {},

        // 5-4
        std::vector<uint8_t> {},

        // 6-1
        std::vector<uint8_t> {
        // header
        0x52, 0x21, 0x16,
            // tile data
            0x0F, 0x20, 0x6E, 0x44, 0x0C, 0xF1, 0x4C, 0x01, 0xAA, 0x35,
            0xD9, 0x34, 0xEE, 0x20, 0x08, 0xB3, 0x37, 0x32, 0x43, 0x04,
            0x4E, 0x21, 0x53, 0x20, 0x7C, 0x01, 0x97, 0x21, 0xB7, 0x07,
            0x9C, 0x81, 0xE7, 0x42, 0x5F, 0xB3, 0x97, 0x63, 0xAC, 0x02,
            0xC5, 0x41, 0x49, 0xE0, 0x58, 0x61, 0x76, 0x64, 0x85, 0x65,
            0x94, 0x66, 0xA4, 0x22, 0xA6, 0x03, 0xC8, 0x22, 0xDC, 0x02,
            0x68, 0xF2, 0x96, 0x42, 0x13, 0x82, 0x17, 0x02, 0xAF, 0x34,
            0xF6, 0x21, 0xFC, 0x06, 0x26, 0x80, 0x2A, 0x24, 0x36, 0x01,
            0x8C, 0x00, 0xFF, 0x35, 0x4E, 0xA0, 0x55, 0x21, 0x77, 0x20,
            0x87, 0x07, 0x89, 0x22, 0xAE, 0x21, 0x4C, 0x82, 0x9F, 0x34,
            0xEC, 0x01, 0x03, 0xE7, 0x13, 0x67, 0x8D, 0x4A, 0xAD, 0x41,
            0x0F, 0xA6,
            0xFDu
    },

        // 6-2
        std::vector<uint8_t> {
        // header
        0x54, 0x21, 0x16,
            // tile data
            0x0F, 0x26, 0xA7, 0x22, 0x37, 0xFB, 0x73, 0x20, 0x83, 0x07,
            0x87, 0x02, 0x93, 0x20, 0xC7, 0x73, 0x04, 0xF1, 0x06, 0x31,
            0x39, 0x71, 0x59, 0x71, 0xE7, 0x73, 0x37, 0xA0, 0x47, 0x04,
            0x86, 0x7C, 0xE5, 0x71, 0xE7, 0x31, 0x33, 0xA4, 0x39, 0x71,
            0xA9, 0x71, 0xD3, 0x23, 0x08, 0xF2, 0x13, 0x05, 0x27, 0x02,
            0x49, 0x71, 0x75, 0x75, 0xE8, 0x72, 0x67, 0xF3, 0x99, 0x71,
            0xE7, 0x20, 0xF4, 0x72, 0xF7, 0x31, 0x17, 0xA0, 0x33, 0x20,
            0x39, 0x71, 0x73, 0x28, 0xBC, 0x05, 0x39, 0xF1, 0x79, 0x71,
            0xA6, 0x21, 0xC3, 0x06, 0xD3, 0x20, 0xDC, 0x00, 0xFC, 0x00,
            0x07, 0xA2, 0x13, 0x21, 0x5F, 0x32, 0x8C, 0x00, 0x98, 0x7A,
            0xC7, 0x63, 0xD9, 0x61, 0x03, 0xA2, 0x07, 0x22, 0x74, 0x72,
            0x77, 0x31, 0xE7, 0x73, 0x39, 0xF1, 0x58, 0x72, 0x77, 0x73,
            0xD8, 0x72, 0x7F, 0xB1, 0x97, 0x73, 0xB6, 0x64, 0xC5, 0x65,
            0xD4, 0x66, 0xE3, 0x67, 0xF3, 0x67, 0x8D, 0xC1, 0xCF, 0x26,
            0xFDu
    },

        // 6-3
        std::vector<uint8_t> {
        // header
        0x97, 0x11, 0x16,
            // tile data
            0x0F, 0x26, 0xFE, 0x10, 0x2B, 0x92, 0x57, 0x12, 0x8B, 0x12,
            0xC0, 0x41, 0xF7, 0x13, 0x5B, 0x92, 0x69, 0x0B, 0xBB, 0x12,
            0xB2, 0x46, 0x19, 0x93, 0x71, 0x00, 0x17, 0x94, 0x7C, 0x14,
            0x7F, 0x11, 0x93, 0x41, 0xBF, 0x15, 0xFC, 0x13, 0xFF, 0x11,
            0x2F, 0x95, 0x50, 0x42, 0x51, 0x12, 0x58, 0x14, 0xA6, 0x12,
            0xDB, 0x12, 0x1B, 0x93, 0x46, 0x43, 0x7B, 0x12, 0x8D, 0x49,
            0xB7, 0x14, 0x1B, 0x94, 0x49, 0x0B, 0xBB, 0x12, 0xFC, 0x13,
            0xFF, 0x12, 0x03, 0xC1, 0x2F, 0x15, 0x43, 0x12, 0x4B, 0x13,
            0x77, 0x13, 0x9D, 0x4A, 0x15, 0xC1, 0xA1, 0x41, 0xC3, 0x12,
            0xFE, 0x01, 0x7D, 0xC1, 0x9E, 0x42, 0xCF, 0x20,
            0xFDu
    },

        // 6-4
        std::vector<uint8_t> {},

        // 7-1
        std::vector<uint8_t> {
        // header
        0x52, 0xB1, 0x16,
            // tile data
            0x0F, 0x20, 0x6E, 0x45, 0x39, 0x91, 0xB3, 0x04, 0xC3, 0x21,
            0xC8, 0x11, 0xCA, 0x10, 0x49, 0x91, 0x7C, 0x73, 0xE8, 0x12,
            0x88, 0x91, 0x8A, 0x10, 0xE7, 0x21, 0x05, 0x91, 0x07, 0x30,
            0x17, 0x07, 0x27, 0x20, 0x49, 0x11, 0x9C, 0x01, 0xC8, 0x72,
            0x23, 0xA6, 0x27, 0x26, 0xD3, 0x03, 0xD8, 0x7A, 0x89, 0x91,
            0xD8, 0x72, 0x39, 0xF1, 0xA9, 0x11, 0x09, 0xF1, 0x63, 0x24,
            0x67, 0x24, 0xD8, 0x62, 0x28, 0x91, 0x2A, 0x10, 0x56, 0x21,
            0x70, 0x04, 0x79, 0x0B, 0x8C, 0x00, 0x94, 0x21, 0x9F, 0x35,
            0x2F, 0xB8, 0x3D, 0xC1, 0x7F, 0x26,
            0xFDu
    },

        // 7-2
        std::vector<uint8_t> {},

        // 7-3
        std::vector<uint8_t> {},

        // 7-4
        std::vector<uint8_t> {
        // header
        0x5B, 0x07, 0x30,
            // tile data
            0x05, 0x32, 0x06, 0x33, 0x07, 0x34, 0xFE, 0x0A, 0xAE, 0x86,
            0xBE, 0x07, 0xFE, 0x02, 0x0D, 0x02, 0x27, 0x32, 0x46, 0x61,
            0x55, 0x62, 0x5E, 0x0E, 0x1E, 0x82, 0x68, 0x3C, 0x74, 0x3A,
            0x7D, 0x4B, 0x5E, 0x8E, 0x7D, 0x4B, 0x7E, 0x82, 0x84, 0x62,
            0x94, 0x61, 0xA4, 0x31, 0xBD, 0x4B, 0xCE, 0x06, 0xFE, 0x02,
            0x0D, 0x06, 0x34, 0x31, 0x3E, 0x0A, 0x64, 0x32, 0x75, 0x0A,
            0x7B, 0x61, 0xA4, 0x33, 0xAE, 0x02, 0xDE, 0x0E, 0x3E, 0x82,
            0x64, 0x32, 0x78, 0x32, 0xB4, 0x36, 0xC8, 0x36, 0xDD, 0x4B,
            0x44, 0xB2, 0x58, 0x32, 0x94, 0x63, 0xA4, 0x3E, 0xBA, 0x30,
            0xC9, 0x61, 0xCE, 0x06, 0xDD, 0x4B, 0xCE, 0x86, 0xDD, 0x4B,
            0xFE, 0x02, 0x2E, 0x86, 0x5E, 0x02, 0x7E, 0x06, 0xFE, 0x02,
            0x1E, 0x86, 0x3E, 0x02, 0x5E, 0x06, 0x7E, 0x02, 0x9E, 0x06,
            0xFE, 0x0A, 0x0D, 0xC4, 0xCD, 0x43, 0xCE, 0x09, 0xDE, 0x0B,
            0xDD, 0x42, 0xFE, 0x02, 0x5D, 0xC7,
            0xFDu
    },

        // 8-1
        std::vector<uint8_t> {
        // header
        0x92, 0x31, 0x10,
            // tile data
            0x0F, 0x20, 0x6E, 0x40, 0x0D, 0x02, 0x37, 0x73, 0xEC, 0x00,
            0x0C, 0x80, 0x3C, 0x00, 0x6C, 0x00, 0x9C, 0x00, 0x06, 0xC0,
            0xC7, 0x73, 0x06, 0x83, 0x28, 0x72, 0x96, 0x40, 0xE7, 0x73,
            0x26, 0xC0, 0x87, 0x7B, 0xD2, 0x41, 0x39, 0xF1, 0xC8, 0xF2,
            0x97, 0xE3, 0xA3, 0x23, 0xE7, 0x02, 0xE3, 0x07, 0xF3, 0x22,
            0x37, 0xE3, 0x9C, 0x00, 0xBC, 0x00, 0xEC, 0x00, 0x0C, 0x80,
            0x3C, 0x00, 0x86, 0x21, 0xA6, 0x06, 0xB6, 0x24, 0x5C, 0x80,
            0x7C, 0x00, 0x9C, 0x00, 0x29, 0xE1, 0xDC, 0x05, 0xF6, 0x41,
            0xDC, 0x80, 0xE8, 0x72, 0x0C, 0x81, 0x27, 0x73, 0x4C, 0x01,
            0x66, 0x74, 0x0D, 0x11, 0x3F, 0x35, 0xB6, 0x41, 0x2C, 0x82,
            0x36, 0x40, 0x7C, 0x02, 0x86, 0x40, 0xF9, 0x61, 0x39, 0xE1,
            0xAC, 0x04, 0xC6, 0x41, 0x0C, 0x83, 0x16, 0x41, 0x88, 0xF2,
            0x39, 0xF1, 0x7C, 0x00, 0x89, 0x61, 0x9C, 0x00, 0xA7, 0x63,
            0xBC, 0x00, 0xC5, 0x65, 0xDC, 0x00, 0xE3, 0x67, 0xF3, 0x67,
            0x8D, 0xC1, 0xCF, 0x26,
            0xFDu
    },

        // 8-2
        std::vector<uint8_t> {
        // header
        0x50, 0xB1, 0x10,
            // tile data
            0x0F, 0x26, 0xFC, 0x00, 0x1F, 0xB3, 0x5C, 0x00, 0x65, 0x65,
            0x74, 0x66, 0x83, 0x67, 0x93, 0x67, 0xDC, 0x73, 0x4C, 0x80,
            0xB3, 0x20, 0xC9, 0x0B, 0xC3, 0x08, 0xD3, 0x2F, 0xDC, 0x00,
            0x2C, 0x80, 0x4C, 0x00, 0x8C, 0x00, 0xD3, 0x2E, 0xED, 0x4A,
            0xFC, 0x00, 0xD7, 0xA1, 0xEC, 0x01, 0x4C, 0x80, 0x59, 0x11,
            0xD8, 0x11, 0xDA, 0x10, 0x37, 0xA0, 0x47, 0x04, 0x99, 0x11,
            0xE7, 0x21, 0x3A, 0x90, 0x67, 0x20, 0x76, 0x10, 0x77, 0x60,
            0x87, 0x07, 0xD8, 0x12, 0x39, 0xF1, 0xAC, 0x00, 0xE9, 0x71,
            0x0C, 0x80, 0x2C, 0x00, 0x4C, 0x05, 0xC7, 0x7B, 0x39, 0xF1,
            0xEC, 0x00, 0xF9, 0x11, 0x0C, 0x82, 0x6F, 0x34, 0xF8, 0x11,
            0xFA, 0x10, 0x7F, 0xB2, 0xAC, 0x00, 0xB6, 0x64, 0xCC, 0x01,
            0xE3, 0x67, 0xF3, 0x67, 0x8D, 0xC1, 0xCF, 0x26,
            0xFDu
    },

        // 8-3
        std::vector<uint8_t> {
        // header
        0x90, 0xB1, 0x10,
            // tile data
            0x0F, 0x26, 0x29, 0x91, 0x7E, 0x42, 0xFE, 0x40, 0x28, 0x92,
            0x4E, 0x42, 0x2E, 0xC0, 0x57, 0x73, 0xC3, 0x25, 0xC7, 0x27,
            0x23, 0x84, 0x33, 0x20, 0x5C, 0x01, 0x77, 0x63, 0x88, 0x62,
            0x99, 0x61, 0xAA, 0x60, 0xBC, 0x01, 0xEE, 0x42, 0x4E, 0xC0,
            0x69, 0x11, 0x7E, 0x42, 0xDE, 0x40, 0xF8, 0x62, 0x0E, 0xC2,
            0xAE, 0x40, 0xD7, 0x63, 0xE7, 0x63, 0x33, 0xA7, 0x37, 0x27,
            0x43, 0x04, 0xCC, 0x01, 0xE7, 0x73, 0x0C, 0x81, 0x3E, 0x42,
            0x0D, 0x0A, 0x5E, 0x40, 0x88, 0x72, 0xBE, 0x42, 0xE7, 0x87,
            0xFE, 0x40, 0x39, 0xE1, 0x4E, 0x00, 0x69, 0x60, 0x87, 0x60,
            0xA5, 0x60, 0xC3, 0x31, 0xFE, 0x31, 0x6D, 0xC1, 0xBE, 0x42,
            0xEF, 0x20,
            0xFDu
    },

        // 8-4
        std::vector<uint8_t> {
        // header
        0x5B, 0x06, 0x30,
            // tile data
            0x05, 0x32, 0x06, 0x33, 0x07, 0x34, 0x5E, 0x0A, 0xAE, 0x02,
            0x0D, 0x01, 0x39, 0x73, 0x0D, 0x03, 0x39, 0x7B, 0x4D, 0x4B,
            0xDE, 0x06, 0x1E, 0x8A, 0xAE, 0x06, 0xC4, 0x33, 0x16, 0xFE,
            0xA5, 0x77, 0xFE, 0x02, 0xFE, 0x82, 0x0D, 0x07, 0x39, 0x73,
            0xA8, 0x74, 0xED, 0x4B, 0x49, 0xFB, 0xE8, 0x74, 0xFE, 0x0A,
            0x2E, 0x82, 0x67, 0x02, 0x84, 0x7A, 0x87, 0x31, 0x0D, 0x0B,
            0xFE, 0x02, 0x0D, 0x0C, 0x39, 0x73, 0x5E, 0x06, 0xC6, 0x76,
            0x45, 0xFF, 0xBE, 0x0A, 0xDD, 0x48, 0xFE, 0x06, 0x3D, 0xCB,
            0x46, 0x7E, 0xAD, 0x4A, 0xFE, 0x82, 0x39, 0xF3, 0xA9, 0x7B,
            0x4E, 0x8A, 0x9E, 0x07, 0xFE, 0x0A, 0x0D, 0xC4, 0xCD, 0x43,
            0xCE, 0x09, 0xDE, 0x0B, 0xDD, 0x42, 0xFE, 0x02, 0x5D, 0xC7,
            0xFDu
    }
    };

    return Data[areaPointer].data();
}

const uint8_t* GetBadGuysData(const uint8_t& areaPointer) {
    static std::vector<uint8_t> Data[] = {
        // 1-1
        std::vector<uint8_t> {
        0x1E, 0xC2, 0x00, 0x6B, 0x06, 0x8B, 0x86, 0x63, 0xB7, 0x0F, 0x05,
            0x03, 0x06, 0x23, 0x06, 0x4B, 0xB7, 0xBB, 0x00, 0x5B, 0xB7,
            0xFB, 0x37, 0x3B, 0xB7, 0x0F, 0x0B, 0x1B, 0x37,
            0xFF
    },

        // 1-2
        std::vector<uint8_t> {
        0x0B, 0x86, 0x1A, 0x06, 0xDB, 0x06, 0xDE, 0xC2, 0x02, 0xF0, 0x3B,
            0xBB, 0x80, 0xEB, 0x06, 0x0B, 0x86, 0x93, 0x06, 0xF0, 0x39,
            0x0F, 0x06, 0x60, 0xB8, 0x1B, 0x86, 0xA0, 0xB9, 0xB7, 0x27,
            0xBD, 0x27, 0x2B, 0x83, 0xA1, 0x26, 0xA9, 0x26, 0xEE, 0x25, 0x0B,
            0x27, 0xB4,
            0xFF
    },

        // 1-3
        std::vector<uint8_t> {
        0x2B, 0xD7, 0xE3, 0x03, 0xC2, 0x86, 0xE2, 0x06, 0x76, 0xA5,
            0xA3, 0x8F, 0x03, 0x86, 0x2B, 0x57, 0x68, 0x28, 0xE9, 0x28,
            0xE5, 0x83, 0x24, 0x8F, 0x36, 0xA8, 0x5B, 0x03,
            0xFF
    },

        // 1-4
        std::vector<uint8_t> {
        0x76, 0xDD, 0xBB, 0x4C, 0xEA, 0x1D, 0x1B, 0xCC, 0x56, 0x5D,
            0x16, 0x9D, 0xC6, 0x1D, 0x36, 0x9D, 0xC9, 0x1D, 0x04, 0xDB,
            0x49, 0x1D, 0x84, 0x1B, 0xC9, 0x5D, 0x88, 0x95, 0x0F, 0x08,
            0x30, 0x4C, 0x78, 0x2D, 0xA6, 0x28, 0x90, 0xB5,
            0xFF
    },

        // 2-1
        std::vector<uint8_t> {
        0x85, 0x86, 0x0B, 0x80, 0x1B, 0x00, 0xDB, 0x37, 0x77, 0x80,
            0xEB, 0x37, 0xFE, 0x2B, 0x20, 0x2B, 0x80, 0x7B, 0x38, 0xAB, 0xB8,
            0x77, 0x86, 0xFE, 0x42, 0x20, 0x49, 0x86, 0x8B, 0x06, 0x9B, 0x80,
            0x7B, 0x8E, 0x5B, 0xB7, 0x9B, 0x0E, 0xBB, 0x0E, 0x9B, 0x80,
            0xFF
    },

        // 2-2
        std::vector<uint8_t> {
        0x0F, 0x01, 0x2E, 0x25, 0x2B, 0x2E, 0x25, 0x4B, 0x4E, 0x25, 0xCB, 0x6B, 0x07,
            0x97, 0x47, 0xE9, 0x87, 0x47, 0xC7, 0x7A, 0x07, 0xD6, 0xC7,
            0x78, 0x07, 0x38, 0x87, 0xAB, 0x47, 0xE3, 0x07, 0x9B, 0x87,
            0x0F, 0x09, 0x68, 0x47, 0xDB, 0xC7, 0x3B, 0xC7,
            0xFF
    },

        // 2-3
        std::vector<uint8_t> {
        0x0F, 0x02, 0x78, 0x40, 0x48, 0xCE, 0xF8, 0xC3, 0xF8, 0xC3,
            0x0F, 0x07, 0x7B, 0x43, 0xC6, 0xD0, 0x0F, 0x8A, 0xC8, 0x50,
            0xFF
    },

        // 2-4
        std::vector<uint8_t> {
        0x0B, 0x8C, 0x4B, 0x4C, 0x77, 0x5F, 0xEB, 0x0C, 0xBD, 0xDB,
            0x19, 0x9D, 0x75, 0x1D, 0x7D, 0x5B, 0xD9, 0x1D, 0x3D, 0xDD,
            0x99, 0x1D, 0x26, 0x9D, 0x5A, 0x2B, 0x8A, 0x2C, 0xCA, 0x1B,
            0x20, 0x95, 0x7B, 0x5C, 0xDB, 0x4C, 0x1B, 0xCC, 0x3B, 0xCC,
            0x78, 0x2D, 0xA6, 0x28, 0x90, 0xB5,
            0xFF
    },

        // 3-1
        std::vector<uint8_t> {
        0x9B, 0x8E, 0xCA, 0x0E, 0xEE, 0x42, 0x44, 0x5B, 0x86, 0x80, 0xB8,
            0x1B, 0x80, 0x50, 0xBA, 0x10, 0xB7, 0x5B, 0x00, 0x17, 0x85,
            0x4B, 0x05, 0xFE, 0x34, 0x40, 0xB7, 0x86, 0xC6, 0x06, 0x5B, 0x80,
            0x83, 0x00, 0xD0, 0x38, 0x5B, 0x8E, 0x8A, 0x0E, 0xA6, 0x00,
            0xBB, 0x0E, 0xC5, 0x80, 0xF3, 0x00,
            0xFF
    },

        // 3-2
        std::vector<uint8_t> {
        0x1B, 0x80, 0xBB, 0x38, 0x4B, 0xBC, 0xEB, 0x3B, 0x0F, 0x04,
            0x2B, 0x00, 0xAB, 0x38, 0xEB, 0x00, 0xCB, 0x8E, 0xFB, 0x80,
            0xAB, 0xB8, 0x6B, 0x80, 0xFB, 0x3C, 0x9B, 0xBB, 0x5B, 0xBC,
            0xFB, 0x00, 0x6B, 0xB8, 0xFB, 0x38,
            0xFF
    },

        // 3-3
        std::vector<uint8_t> {
        0xA5, 0x86, 0xE4, 0x28, 0x18, 0xA8, 0x45, 0x83, 0x69, 0x03,
            0xC6, 0x29, 0x9B, 0x83, 0x16, 0xA4, 0x88, 0x24, 0xE9, 0x28,
            0x05, 0xA8, 0x7B, 0x28, 0x24, 0x8F, 0xC8, 0x03, 0xE8, 0x03,
            0x46, 0xA8, 0x85, 0x24, 0xC8, 0x24,
            0xFF
    },

        // 3-4
        std::vector<uint8_t> {
        0x0B, 0x8C, 0x3B, 0x1D, 0x8B, 0x1D, 0xAB, 0x0C, 0xDB, 0x1D,
            0x0F, 0x03, 0x65, 0x1D, 0x6B, 0x1B, 0x05, 0x9D, 0x0B, 0x1B,
            0x05, 0x9B, 0x0B, 0x1D, 0x8B, 0x0C, 0x1B, 0x8C, 0x70, 0x15,
            0x7B, 0x0C, 0xDB, 0x0C, 0x0F, 0x08, 0x78, 0x2D, 0xA6, 0x28,
            0x90, 0xB5,
            0xFF
    },

        // 4-1
        std::vector<uint8_t> {
        0x2E, 0xC2, 0x66, 0xE2, 0x11, 0x0F, 0x07, 0x02, 0x11, 0x0F, 0x0C,
            0x12, 0x11,
            0xFF
    },

        // 4-2
        std::vector<uint8_t> {
        0x0F, 0x02, 0x1E, 0x2F, 0x60, 0xE0, 0x3A, 0xA5, 0xA7, 0xDB, 0x80,
            0x3B, 0x82, 0x8B, 0x02, 0xFE, 0x42, 0x68, 0x70, 0xBB, 0x25, 0xA7,
            0x2C, 0x27, 0xB2, 0x26, 0xB9, 0x26, 0x9B, 0x80, 0xA8, 0x82,
            0xB5, 0x27, 0xBC, 0x27, 0xB0, 0xBB, 0x3B, 0x82, 0x87, 0x34,
            0xEE, 0x25, 0x6B,
            0xFF
    },

        // 4-3
        std::vector<uint8_t> {
        0xC7, 0x83, 0xD7, 0x03, 0x42, 0x8F, 0x7A, 0x03, 0x05, 0xA4,
            0x78, 0x24, 0xA6, 0x25, 0xE4, 0x25, 0x4B, 0x83, 0xE3, 0x03,
            0x05, 0xA4, 0x89, 0x24, 0xB5, 0x24, 0x09, 0xA4, 0x65, 0x24,
            0xC9, 0x24, 0x0F, 0x08, 0x85, 0x25,
            0xFF
    },

        // 4-4
        std::vector<uint8_t> {
        0x0F, 0x03, 0x56, 0x1B, 0xC9, 0x1B, 0x0F, 0x07, 0x36, 0x1B,
            0xAA, 0x1B, 0x48, 0x95, 0x0F, 0x0A, 0x2A, 0x1B, 0x5B, 0x0C,
            0x78, 0x2D, 0x90, 0xB5,
            0xFF
    },

        // 5-1
        std::vector<uint8_t> {
        0x0B, 0x80, 0x60, 0x38, 0x10, 0xB8, 0xC0, 0x3B, 0xDB, 0x8E,
            0x40, 0xB8, 0xF0, 0x38, 0x7B, 0x8E, 0xA0, 0xB8, 0xC0, 0xB8,
            0xFB, 0x00, 0xA0, 0xB8, 0x30, 0xBB, 0xEE, 0x42, 0x88, 0x0F, 0x0B,
            0x2B, 0x0E, 0x67, 0x0E,
            0xFF
    },

        // 5-2
        std::vector<uint8_t> {
        0x7B, 0x80, 0xAE, 0x00, 0x80, 0x8B, 0x8E, 0xE8, 0x05, 0xF9, 0x86,
            0x17, 0x86, 0x16, 0x85, 0x4E, 0x2B, 0x80, 0xAB, 0x8E, 0x87, 0x85,
            0xC3, 0x05, 0x8B, 0x82, 0x9B, 0x02, 0xAB, 0x02, 0xBB, 0x86,
            0xCB, 0x06, 0xD3, 0x03, 0x3B, 0x8E, 0x6B, 0x0E, 0xA7, 0x8E,
            0xFF
    },

        // 5-3
        std::vector<uint8_t> {},

        // 5-4
        std::vector<uint8_t> {},

        // 6-1
        std::vector<uint8_t> {
        0x0F, 0x02, 0x02, 0x11, 0x0F, 0x07, 0x02, 0x11,
            0xFF
    },

        // 6-2
        std::vector<uint8_t> {
        0x0E, 0xC2, 0xA8, 0xAB, 0x00, 0xBB, 0x8E, 0x6B, 0x82, 0xDE, 0x00, 0xA0,
            0x33, 0x86, 0x43, 0x06, 0x3E, 0xB4, 0xA0, 0xCB, 0x02, 0x0F, 0x07,
            0x7E, 0x42, 0xA6, 0x83, 0x02, 0x0F, 0x0A, 0x3B, 0x02, 0xCB, 0x37,
            0x0F, 0x0C, 0xE3, 0x0E,
            0xFF
    },

        // 6-3
        std::vector<uint8_t> {
        0xCD, 0xA5, 0xB5, 0xA8, 0x07, 0xA8, 0x76, 0x28, 0xCC, 0x25,
            0x65, 0xA4, 0xA9, 0x24, 0xE5, 0x24, 0x19, 0xA4, 0x0F, 0x07,
            0x95, 0x28, 0xE6, 0x24, 0x19, 0xA4, 0xD7, 0x29, 0x16, 0xA9,
            0x58, 0x29, 0x97, 0x29,
            0xFF
    },

        // 6-4
        std::vector<uint8_t> {},

        // 7-1
        std::vector<uint8_t> {
        0xAB, 0xCE, 0xDE, 0x42, 0xC0, 0xCB, 0xCE, 0x5B, 0x8E, 0x1B, 0xCE,
            0x4B, 0x85, 0x67, 0x45, 0x0F, 0x07, 0x2B, 0x00, 0x7B, 0x85,
            0x97, 0x05, 0x0F, 0x0A, 0x92, 0x02,
            0xFF
    },

        // 7-2
        std::vector<uint8_t> {},

        // 7-3
        std::vector<uint8_t> {},

        // 7-4
        std::vector<uint8_t> {
        0x27, 0xA9, 0x4B, 0x0C, 0x68, 0x29, 0x0F, 0x06, 0x77, 0x1B,
            0x0F, 0x0B, 0x60, 0x15, 0x4B, 0x8C, 0x78, 0x2D, 0x90, 0xB5,
            0xFF
    },

        // 8-1
        std::vector<uint8_t> {
        0x2B, 0x82, 0xAB, 0x38, 0xDE, 0x42, 0xE2, 0x1B, 0xB8, 0xEB,
            0x3B, 0xDB, 0x80, 0x8B, 0xB8, 0x1B, 0x82, 0xFB, 0xB8, 0x7B,
            0x80, 0xFB, 0x3C, 0x5B, 0xBC, 0x7B, 0xB8, 0x1B, 0x8E, 0xCB,
            0x0E, 0x1B, 0x8E, 0x0F, 0x0D, 0x2B, 0x3B, 0xBB, 0xB8, 0xEB, 0x82,
            0x4B, 0xB8, 0xBB, 0x38, 0x3B, 0xB7, 0xBB, 0x02, 0x0F, 0x13,
            0x1B, 0x00, 0xCB, 0x80, 0x6B, 0xBC,
            0xFF
    },

        // 8-2
        std::vector<uint8_t> {
        0x29, 0x8E, 0x52, 0x11, 0x83, 0x0E, 0x0F, 0x03, 0x9B, 0x0E,
            0x2B, 0x8E, 0x5B, 0x0E, 0xCB, 0x8E, 0xFB, 0x0E, 0xFB, 0x82,
            0x9B, 0x82, 0xBB, 0x02, 0xFE, 0x42, 0xE8, 0xBB, 0x8E, 0x0F, 0x0A,
            0xAB, 0x0E, 0xCB, 0x0E, 0xF9, 0x0E, 0x88, 0x86, 0xA6, 0x06,
            0xDB, 0x02, 0xB6, 0x8E,
            0xFF
    },

        // 8-3
        std::vector<uint8_t> {
        0xEB, 0x8E, 0x0F, 0x03, 0xFB, 0x05, 0x17, 0x85, 0xDB, 0x8E,
            0x0F, 0x07, 0x57, 0x05, 0x7B, 0x05, 0x9B, 0x80, 0x2B, 0x85,
            0xFB, 0x05, 0x0F, 0x0B, 0x1B, 0x05, 0x9B, 0x05,
            0xFF
    },

        // 8-4
        std::vector<uint8_t> {
        0x0F, 0x03, 0x8E, 0x65, 0xE1, 0xBB, 0x38, 0x6D, 0xA8, 0x3E, 0xE5, 0xE7,
            0x0F, 0x08, 0x0B, 0x02, 0x2B, 0x02, 0x5E, 0x65, 0xE1, 0xBB, 0x0E,
            0xDB, 0x0E, 0xBB, 0x8E, 0xDB, 0x0E, 0xFE, 0x65, 0xEC, 0x0F, 0x0D,
            0x4E, 0x65, 0xE1, 0x0F, 0x0E, 0x4E, 0x02, 0xE0, 0x0F, 0x10, 0xFE, 0xE5, 0xE1,
            0x1B, 0x85, 0x7B, 0x0C, 0x5B, 0x95, 0x78, 0x2D, 0x90, 0xB5,
            0xFF
    }
    };

    return Data[areaPointer].data();
}

void MapLoader::extendMap(World& world) {
    for (int i = 0; i < 208u; ++i) {
        world.m_Tiles.push_back(nullptr);
    }

    world.m_AttributeTable.insert(world.m_AttributeTable.end(), 208u, 0u);
}

void MapLoader::onMazeTrigger(uint8_t targetPage) {
    CurrentPage = targetPage;

    TileSkipToPage = 0x00u;
    BadGuysSkipToPage = 0x00u;

    TileDataIterator = 0x03u;
    BadGuysDataIterator = 0x00u;

    CurrentMazeScore = 0x00u;

    LastTileObjectPageFlag = false;
    LastBadGuysObjectPageFlag = false;
    LastBadGuysObjectWasPageSkip = false;

    // setting tile iterator
    while (true) {
        const uint8_t& b1 = TileData[TileDataIterator];

        if (b1 == 0xFDu) {
            break;
        }

        const uint8_t& b2 = TileData[(++TileDataIterator)++];

        if (ObjectIsPageSkip(b1, b2) == targetPage) {
            break;
        }
    }

    // setting bad guys iterator
    while (true) {
        const uint8_t& b1 = BadGuysData[BadGuysDataIterator];

        if (b1 == 0xFFu) {
            break;
        }

        const uint8_t& b2 = BadGuysData[(++BadGuysDataIterator)++];

        const uint8_t yPos = ObjectYPos(b1, b2);

        if (yPos == 0x0Eu) {
            ++BadGuysDataIterator;
        } else if (yPos == 0x0Fu && b2 >= targetPage) {
            BadGuysSkipToPage = b2;
            break;
        }
    }

    BackgroundModifiers = getBackgroundodifiers();
}

void MapLoader::handleMaze(World& world) {
    if (player.Data.GetLevelPointer() == 15u) { // level 4-4
        if (CurrentPage == 5u) {
            if (player.yPosition() >= 65.f) {
                onMazeTrigger(1u);
            }
        } else if (CurrentPage == 9u) {
            if (player.yPosition() < 144.f) {
                onMazeTrigger(5u);
            }
        }
    } else if (player.Data.GetLevelPointer() == 27u) { // level 7-4

    } else if (player.Data.GetLevelPointer() == 31u) { // level 8-4

    }
}

void MapLoader::placeBlock(const unsigned int& tileIndex, std::unique_ptr<Blocks::Block> block, const uint8_t& subPalleteIndex, World& world) {
    world.m_Tiles[tileIndex] = std::move(block);
    world.m_AttributeTable[tileIndex] = std::move(subPalleteIndex);
}

void MapLoader::spawnSprite(std::unique_ptr<Sprite> sprite, World& world) {
    auto it = std::lower_bound(
        world.m_SpritePool.begin(), world.m_SpritePool.end(), sprite->Position.x,
        [](const std::vector<std::unique_ptr<Sprite>>& group, const float& xValue) {
        return group.front()->Position.x < xValue;
    }
    );

    world.m_SpritePool.insert(it, std::vector<std::unique_ptr<Sprite>>{})->push_back(std::move(sprite));
}

void MapLoader::placeBlockIfEmpty(const unsigned int& tileIndex, std::unique_ptr<Blocks::Block> block, const uint8_t& subPalleteIndex, World& world) {
    if (!world.m_Tiles[tileIndex]) {
        world.m_Tiles[tileIndex] = std::move(block);
        world.m_AttributeTable[tileIndex] = std::move(subPalleteIndex);
    }
}

void MapLoader::hiddenOneUpLogic(World& world) {
    uint8_t worldNumber = player.Data.World;
    uint8_t levelNumber = player.Data.Level;

    if (worldNumber == 1 && levelNumber == 1) {
        world.m_SpawnOneUp = true;
    } else if (worldNumber < 8u && levelNumber == 3u) {
        // constexpr uint8_t CoinRequirements[] = {21u, 35u, 22u, 27u, 23u, 24u, 35u, 99u};
        constexpr uint8_t CoinRequirements[] = {35u, 22u, 27u, 23u, 24u, 35u}; // removed unnecessary

        world.m_RequiredCoinsForOneUp = CoinRequirements[worldNumber - 2];
    }
}

void MapLoader::loadMapProperties(World& world) {
    const uint8_t& b1 = TileData[0x00u];
    const uint8_t& b2 = TileData[0x01u];
    const uint8_t& b3 = TileData[0x02u];

    /* load time */ {
        if (const uint8_t time = b1 >> 0x06u) {
            world.m_GameTime = (5u - time) * 100u;
        } else {
            world.m_GameTime = 0u;
        }

        Renderer::SetGameTimeRendering(world.m_GameTime > 0u);
    }

    // background type
    CurrentBackgroundType = b1 & 0x07u;

    // map type
    MapType = b2 >> 0x06u;

    // scenery type
    CurrentSceneryType = (b2 >> 0x04u) & 0x03u;

    // terrain type
    CurrentTerrainType = b2 & 0x0Fu;

    /* attribute type */ {
        const uint8_t mapAttribute = b3 >> 0x04u;

        world.CurrentTheme = mapAttribute;
    }

    // halfway page
    world.HalfwayPage = b3 & 0x0Fu;

    // enable swimming physics
    player.m_SwimmingPhysics = world.CurrentTheme == 0x00u;

    // player position
    constexpr int8_t PlayerPositions[] = {
        -1, 1u, 10u, 4u
    };

    uint8_t posIdx = (b1 >> 3u) & 0x03u;

    player.Position = sf::Vector2f(2.5f, PlayerPositions[posIdx] + 1.f) * TileSize;

    if (posIdx <= 0x01u) {
        world.StartCutscene(std::make_unique<PlayerSpawnScene>(world, posIdx));
    } else if ((b1 >> 5u) & 0x01u) {
        world.StartCutscene(std::make_unique<AutowalkScene>(world));
    }
}

void MapLoader::setTheme(const uint8_t& mapAttribute) {
    uint8_t colorBackground = TileData[0x00u] & 0x07u;

    std::vector<std::pair<unsigned int, SceneData>> backgroundModifiersForColor = getBackgroundodifiers();
    for (const auto& modifier : backgroundModifiersForColor) {
        if (modifier.second.background > 0x03) {
            colorBackground = static_cast<uint8_t>(modifier.second.background);
        }
    }

    uint8_t colorAttribute = colorBackground == 7u ? 3u : mapAttribute;

    bool skyColIndex;
    uint8_t folliageColsIndex = colorAttribute;
    uint8_t bodyColsIndex = colorAttribute;

    if (colorBackground == 0x04u || colorBackground == 0x06u || colorBackground == 0x07u) {
        skyColIndex = false;
    } else if (colorBackground == 0x05u) {
        skyColIndex = true;
    } else if (colorAttribute < 0x02u) {
        skyColIndex = true;
    } else {
        skyColIndex = false;
    }

    if (MapType == 0x01u) {
        folliageColsIndex = 4u;
    } else if (colorBackground == 0x05u || colorBackground == 0x06u) {
        folliageColsIndex = 3u;
    }

    Renderer::SetBackgroundTheme(skyColIndex, folliageColsIndex, bodyColsIndex);
    Renderer::SetSpriteTheme(colorAttribute);
}

void MapLoader::handleCellingTerrain(const uint8_t& length, const uint8_t& block, const unsigned int& colIndex, World& world) {
    for (uint8_t i = 0u; i < length; ++i) {
        unsigned int index = colIndex + i;

        if (!world.m_Tiles[index]) {
            if (block == gbl::TextureId::Block::Brick_1 || block == gbl::TextureId::Block::Brick_2) {
                placeBlock(index, std::make_unique<Blocks::RenderableCollideableBreakable>(block), GetSubPalleteIndex(block), world);
            } else {
                placeBlock(index, std::make_unique<Blocks::RenderableCollideable>(block), GetSubPalleteIndex(block), world);
            }
        }
    }
}

void MapLoader::handleMiddleTerrain(const uint8_t& length, const uint8_t& block, const unsigned int& colIndex, World& world) {
    for (uint8_t i = 0u; i < length; ++i) {
        unsigned int index = colIndex + 8u - length + i;

        if (!world.m_Tiles[index]) {
            if (block == gbl::TextureId::Block::Brick_1 || block == gbl::TextureId::Block::Brick_2) {
                placeBlock(index, std::make_unique<Blocks::RenderableCollideableBreakable>(block), GetSubPalleteIndex(block), world);
            } else {
                placeBlock(index, std::make_unique<Blocks::RenderableCollideable>(block), GetSubPalleteIndex(block), world);
            }
        }
    }
}

void MapLoader::handleSurfaceTerrain(const uint8_t& length, const uint8_t& block_1, const uint8_t& block_2, const unsigned int& colIndex, World& world) {
    if (length >= 0x02u) {
        for (uint8_t i = 0u; i < 2u; ++i) {
            unsigned int index = colIndex + 11u + i;

            if (!world.m_Tiles[index]) {
                placeBlock(index, std::make_unique<Blocks::RenderableCollideable>(block_2), GetSubPalleteIndex(block_2), world);
            }
        }
    }

    if (length > 0x02u) {
        for (uint8_t i = 0u; i < length - 2; ++i) {
            unsigned int index = colIndex + 13u - length + i;

            if (!world.m_Tiles[index]) {
                placeBlock(index, std::make_unique<Blocks::RenderableCollideable>(block_1), GetSubPalleteIndex(block_1), world);
            }
        }
    }
}

void MapLoader::handleInWaterBackground(const unsigned int& colIndex, World& world) {
    placeBlockIfEmpty(colIndex, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Liquid_1), 2u, world);

    for (uint8_t i = 0u; i < 10u; ++i) {
        placeBlockIfEmpty(colIndex + 1u + i, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Liquid_2), 2u, world);
    }
}

void MapLoader::handleWallBackground(const unsigned int& colIndex, World& world) {
    {
        const unsigned int roofIndex = colIndex + 5u;

        Components::Render* renderComponent = GetComponent(world.m_Tiles[roofIndex].get(), Components::Render);

        if (
            !renderComponent || (
                renderComponent->TextureId != gbl::TextureId::Castle_2
                )
        ) {
            placeBlock(roofIndex, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_1), 1u, world);
        }
    }

    const unsigned int brickIndex = colIndex + 6u;

    for (uint8_t i = 0u; i < 5u; ++i) {
        unsigned int index = brickIndex + i;

        Components::Render* renderComponent = GetComponent(world.m_Tiles[index].get(), Components::Render);

        if (
            !renderComponent || (
                renderComponent->TextureId != gbl::TextureId::Block::Castle_3 &&
                renderComponent->TextureId != gbl::TextureId::Block::Castle_4
                )
        ) {
            placeBlock(index, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
        }
    }
}

void MapLoader::handleOverWaterBackground(const unsigned int& colIndex, World& world) {
    placeBlockIfEmpty(colIndex + 11u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Liquid_1), 2u, world);
    placeBlockIfEmpty(colIndex + 12u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Liquid_2), 2u, world);
}

void MapLoader::placeHorizontalAir(const unsigned int& tileIndex, const uint8_t& length, World& world) {
    for (uint8_t i = 0u; i < length; ++i) {
        world.m_Tiles[tileIndex + i * 13u].reset();
    }
}

void MapLoader::placeVerticalAir(const unsigned int& tileIndex, const uint8_t& length, World& world) {
    for (uint8_t i = 0u; i < length; ++i) {
        world.m_Tiles[tileIndex + i].reset();
    }
}

void MapLoader::placeAirHole(const uint8_t& length, const unsigned int& tileIndex, World& world) {
    const unsigned int holeTopleft = tileIndex - 4;

    for (uint8_t i = 0u; i < length; ++i) {
        placeVerticalAir(holeTopleft + i * 13u, 5u, world);
    }
}

void MapLoader::placeWaterHole(const uint8_t& length, const unsigned int& tileIndex, World& world) {
    const unsigned int holeTopleft = tileIndex - 4;

    for (uint8_t i = 0u; i < length; ++i) {
        placeVerticalRow<Blocks::Renderable>(holeTopleft + i * 13u, 5u, 2u, world, gbl::TextureId::Block::Liquid_2);
    }
}

void MapLoader::placeHole(const uint8_t& length, const unsigned int& tileIndex, World& world) {
    uint8_t placeLength = length;

    const unsigned int localColIndex = (tileIndex / 0x0Du) % 0x10u;
    const uint8_t holeRight = localColIndex + length;

    if (holeRight > 0x10u) {
        uint8_t extraLength = holeRight - 0x10;
        placeLength -= extraLength;
        HoleBufferLength = std::move(extraLength);
    }

    if (world.CurrentTheme == 0x00u) {
        return placeWaterHole(placeLength, tileIndex, world);
    } else {
        return placeAirHole(placeLength, tileIndex, world);
    }
}

void MapLoader::placeHoleFilledWithWater(const uint8_t& length, const unsigned int& tileIndex, World& world) {
    placeHorizontalRow<Blocks::Renderable>(tileIndex - 2, length, 2u, world, gbl::TextureId::Block::Liquid_1);
    placeHorizontalRow<Blocks::Renderable>(tileIndex - 1, length, 2u, world, gbl::TextureId::Block::Liquid_2);
    placeHorizontalRow<Blocks::Renderable>(tileIndex, length, 2u, world, gbl::TextureId::Block::Liquid_2);
}

void MapLoader::placeLiftsVerticalRope(const unsigned int& colIndex, World& world) {
    placeVerticalRow<Blocks::Renderable>(colIndex, 13u, 1u, world, gbl::TextureId::Block::String_2);
}

void MapLoader::placeCastle(const uint8_t& length, const unsigned int& colIndex, const uint8_t& xPos, const unsigned int& pageColumnOffset, World& world) {
    const unsigned int tileIndex = colIndex + length;

    if (length <= 0x0Au) {
        placeHorizontalRow<Blocks::Renderable>(tileIndex + 13u, 3u, 1u, world, gbl::TextureId::Block::Castle_1);
        placeVerticalAir(tileIndex, 2u, world);
        placeVerticalAir(tileIndex + 52u, 2u, world);
    }

    if (length <= 0x09u) {
        placeBlock(tileIndex + 14u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_5), 1u, world);
        placeBlock(tileIndex + 27u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
        placeBlock(tileIndex + 40u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_6), 1u, world);
    }

    if (length <= 0x08u) {
        placeBlock(tileIndex + 2u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_1), 1u, world);
        placeHorizontalRow<Blocks::Renderable>(tileIndex + 15u, 3u, 1u, world, gbl::TextureId::Block::Castle_2);
        placeBlock(tileIndex + 54u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_1), 1u, world);
    }

    if (length <= 0x07u) {
        placeHorizontalRow<Blocks::Renderable>(tileIndex + 3u, 2u, 1u, world, gbl::TextureId::Block::Brick_2);
        placeBlock(tileIndex + 29u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_3), 1u, world);
        placeHorizontalRow<Blocks::Renderable>(tileIndex + 42u, 2u, 1u, world, gbl::TextureId::Block::Brick_2);
    }

    if (length <= 0x06u) {
        placeHorizontalRow<Blocks::Renderable>(tileIndex + 4u, 2u, 1u, world, gbl::TextureId::Block::Brick_2);
        placeBlock(tileIndex + 30u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_4), 1u, world);
        placeHorizontalRow<Blocks::Renderable>(tileIndex + 43u, 2u, 1u, world, gbl::TextureId::Block::Brick_2);
    }

    if (length <= 0x05u) {
        placeHorizontalRow<Blocks::Renderable>(tileIndex + 5u, 5u, 1u, world, gbl::TextureId::Block::Castle_2);
    }

    if (length <= 0x04u) {
        placeBlock(tileIndex + 6u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
        placeBlock(tileIndex + 19u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_3), 1u, world);
        placeBlock(tileIndex + 32u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
        placeBlock(tileIndex + 45u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_3), 1u, world);
        placeBlock(tileIndex + 58u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
    }

    if (length <= 0x03u) {
        placeBlock(tileIndex + 7u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
        placeBlock(tileIndex + 20u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_4), 1u, world);
        placeBlock(tileIndex + 33u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
        placeBlock(tileIndex + 46u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_4), 1u, world);
        placeBlock(tileIndex + 59u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
    }

    if (length <= 0x02u) {
        placeHorizontalRow<Blocks::Renderable>(tileIndex + 8u, 5u, 1u, world, gbl::TextureId::Block::Brick_2);
    }

    if (length <= 0x01u) {
        placeBlock(tileIndex + 9u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_3), 1u, world);
        placeBlock(tileIndex + 22u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
        placeBlock(tileIndex + 35u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_3), 1u, world);
        placeBlock(tileIndex + 48u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
        placeBlock(tileIndex + 61u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_3), 1u, world);
    }

    const bool endOfLevelCastle = (colIndex / 16u) > 0x00u;

    if (length == 0x00u) {
        placeBlock(tileIndex + 10u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_4), 1u, world);
        placeBlock(tileIndex + 23u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
        placeBlock(tileIndex + 36u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_4), 1u, world);
        placeBlock(tileIndex + 49u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Brick_2), 1u, world);
        placeBlock(tileIndex + 62u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Castle_4), 1u, world);

        if (endOfLevelCastle) {
            placeBlock(colIndex + 23u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Brick_2), 1u, world);
        }
    }

    if (endOfLevelCastle) {
        placeBlock(colIndex + 49u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Brick_2), 1u, world);

        spawnSprite(std::make_unique<StarFlag>(
            sf::Vector2f(pageColumnOffset + xPos + 2u, 9.f) * TileSize
        ), world);
    }
}

void MapLoader::placeBridge(uint8_t v, const uint8_t& length, const unsigned int& colIndex, World& world) {
    placeHorizontalRow<Blocks::Renderable>(colIndex + v - 1, length, 0u, world, gbl::TextureId::Block::Bridge_top);
    placeHorizontalRow<Blocks::RenderableCollideable>(colIndex + v, length, 1u, world, gbl::TextureId::Block::Bridge_base);
}

void MapLoader::placeStaircase(const uint8_t& length, const uint8_t& metadata, const unsigned int& colIndex, World& world) {
    const unsigned int anchor = colIndex + 10u;

    // length:8, height:9
    if (length == 0x09u && metadata == 0x08u) {
        for (uint8_t i = 0u; i < 8u; ++i) {
            placeVerticalRow<Blocks::RenderableCollideable>(anchor - i + i * 13u, i + 1u, 1u, world, gbl::TextureId::Block::Flag_base);
        }

        placeVerticalRow<Blocks::RenderableCollideable>(colIndex + 107u, 8u, 1u, world, gbl::TextureId::Block::Flag_base);
    }

    // length:n, height:n
    else {
        for (uint8_t i = 0u; i < length; ++i) {
            placeVerticalRow<Blocks::RenderableCollideable>(anchor - i + i * 13u, i + 1u, 1u, world, gbl::TextureId::Block::Flag_base);
        }
    }
}

void MapLoader::placeHorizontalQuestionBlocksCoin(uint8_t v, const uint8_t& length, const unsigned int& colIndex, World& world) {
    placeHorizontalRow<Blocks::RenderableCollideableItem>(colIndex + v, length, 3u, world, gbl::TextureId::Block::Question, gbl::ItemType::Coin);
}

void MapLoader::placeFlagpole(const unsigned int& colIndex, const uint8_t& xPos, const unsigned int& pageColumnOffset, World& world) {
    placeBlock(colIndex, std::make_unique<Blocks::Flag>(gbl::TextureId::Block::Flag_top), 0u, world);
    placeVerticalRow<Blocks::Flag>(colIndex + 1u, 9u, 0u, world, gbl::TextureId::Block::Flag_pole);
    placeBlock(colIndex + 10u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Flag_base), 1u, world);

    spawnSprite(std::make_unique<Flag>(
        sf::Vector2f(pageColumnOffset + xPos - 0.5f, 3.f) * TileSize
    ), world);
}

void MapLoader::placeSidewaysPipe(const unsigned int& tileIndex, World& world) {
    placeBlock(tileIndex, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Pipe_5), 1u, world);
    placeBlock(tileIndex + 1u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Pipe_8), 1u, world);
}

void MapLoader::placeJumpSpring(const unsigned int& tileIndex, const uint8_t& xPos, const unsigned int& pageColumnOffset, World& world) {
    world.m_Tiles[tileIndex] = std::make_unique<Blocks::JumpSpringTrigger>();

    const unsigned int lower = tileIndex + 1u;
    world.m_Tiles[lower] = std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::TrampolineBase);
    world.m_AttributeTable[lower] = 1u;

    spawnSprite(std::make_unique<JumpSpring>((pageColumnOffset + xPos) * TileSize), world);
}

void MapLoader::placeTree(const uint8_t& yPos, const uint8_t& length, const unsigned int& tileIndex, const unsigned int& colIndex, World& world) {
    placeBlock(tileIndex, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::GrassPlatform_1), 0u, world);
    placeHorizontalRow<Blocks::RenderableCollideable>(tileIndex + 13u, length - 2, 0u, world, gbl::TextureId::Block::GrassPlatform_2);
    placeBlock(tileIndex + (length - 1) * 13u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::GrassPlatform_3), 0u, world);

    const unsigned int indexAnchor_1 = colIndex + 13u;

    for (uint8_t y = yPos + 1u; y < 13u; ++y) {
        const unsigned int indexAnchor_2 = indexAnchor_1 + y;

        for (uint8_t i = 0u; i < length - 2; ++i) {
            const unsigned int index = indexAnchor_2 + i * 13u;
            auto& existingTile = world.m_Tiles[index];

            Components::Render* renderComponent = GetComponent(existingTile.get(), Components::Render);

            if (
                !renderComponent || (
                    renderComponent->TextureId != gbl::TextureId::Block::GrassPlatform_1 &&
                    renderComponent->TextureId != gbl::TextureId::Block::GrassPlatform_2 &&
                    renderComponent->TextureId != gbl::TextureId::Block::GrassPlatform_3
                    )
            ) {
                existingTile = std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::GrassPlatform_base);
                world.m_AttributeTable[index] = 1u;
            }
        }
    }
}

void MapLoader::placeMushroom(const uint8_t& yPos, const uint8_t& length, const unsigned int& tileIndex, World& world) {
    placeBlock(tileIndex, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::MushroomPlatform_1), 0u, world);
    placeHorizontalRow<Blocks::RenderableCollideable>(tileIndex + 13u, length - 2, 0u, world, gbl::TextureId::Block::MushroomPlatform_2);
    placeBlock(tileIndex + (length - 1) * 13u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::MushroomPlatform_3), 0u, world);

    const uint8_t middle = (length / 2u) * 13u;
    placeBlock(tileIndex + 1u + middle, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::MushroomPlatform_middle), 1u, world);

    const unsigned int stemPos = tileIndex + 2u + middle;

    for (uint8_t i = 0u; i < 11u - yPos; ++i) {
        const unsigned int index = stemPos + i;
        auto& existingTile = world.m_Tiles[index];

        Components::Render* renderComponent = GetComponent(existingTile.get(), Components::Render);

        if (
            !renderComponent || (
                renderComponent->TextureId != gbl::TextureId::Block::MushroomPlatform_1 &&
                renderComponent->TextureId != gbl::TextureId::Block::MushroomPlatform_2 &&
                renderComponent->TextureId != gbl::TextureId::Block::MushroomPlatform_3
                )
        ) {
            existingTile = std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::MushroomPlatform_base);
            world.m_AttributeTable[index] = 1u;
        }
    }
}

void MapLoader::placeGun(const uint8_t& yPos, const uint8_t& length, const unsigned int& tileIndex, World& world) {
    placeBlock(tileIndex, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::BulletBill_1), 1u, world);

    if (length > 1u) {
        placeBlock(tileIndex + 1u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::BulletBill_2), 1u, world);
    }

    if (length > 2u) {
        for (uint8_t i = 2u; i < length; ++i) {
            if (yPos + i < 13u) {
                placeBlock(tileIndex + i, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::BulletBill_3), 1u, world);
            }
        }
    }
}

void MapLoader::placeIsland(const uint8_t& yPos, const uint8_t& length, const unsigned int& tileIndex, const unsigned int& colIndex, const uint8_t& mapType, World& world) {
    if (mapType == 0x00u || mapType == 0x03u) {
        return placeTree(yPos, length, tileIndex, colIndex, world);
    }

    else if (mapType == 0x01u) {
        return placeMushroom(yPos, length, tileIndex, world);
    }

    else {
        return placeGun(yPos, length, tileIndex, world);
    }
}

void MapLoader::placeLongReverseLPipe(const unsigned int& colIndex, World& world) {
    placeBlock(colIndex + 6u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Pipe_5), 0u, world);
    placeBlock(colIndex + 19u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Pipe_6), 0u, world);
    placeBlock(colIndex + 32u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Pipe_7), 0u, world);
    placeBlock(colIndex + 7u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Pipe_8), 0u, world);
    placeBlock(colIndex + 20u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Pipe_9), 0u, world);
    placeBlock(colIndex + 33u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Pipe_10), 0u, world);

    placeVerticalRow<Blocks::RenderableCollideable>(colIndex + 26u, 6u, 0u, world, gbl::TextureId::Block::Pipe_3);
    placeVerticalRow<Blocks::RenderableCollideable>(colIndex + 39u, 8u, 0u, world, gbl::TextureId::Block::Pipe_4);
}

void MapLoader::placePipe(const uint8_t& length, bool enterable, const unsigned int& tileIndex, const uint8_t& xPos, const uint8_t& yPos, const unsigned int& pageColumnOffset, World& world) {
    if (enterable) {
        placeBlock(tileIndex, std::make_unique<Blocks::RenderableCollideableEnterable>(gbl::TextureId::Block::Pipe_1), 0u, world);
    } else {
        placeBlock(tileIndex, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Pipe_1), 0u, world);
    }

    placeVerticalRow<Blocks::RenderableCollideable>(tileIndex + 1u, length - 1, 0u, world, gbl::TextureId::Block::Pipe_3);

    placeBlock(tileIndex + 13u, std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Pipe_2), 0u, world);

    placeVerticalRow<Blocks::RenderableCollideable>(tileIndex + 14u, length - 1, 0u, world, gbl::TextureId::Block::Pipe_4);

    if (player.Data.GetLevelPointer() > 0x00u) {
        spawnSprite(std::make_unique<PiranhaPlant>(sf::Vector2f(pageColumnOffset + xPos + 0.5f, yPos) * TileSize), world);
    }
}

void MapLoader::NewLevel(World& world, const uint8_t& areaPointer) {
    world.Reset();
    player.Reset();

    hiddenOneUpLogic(world);

    TileData = GetTileData(areaPointer);
    BadGuysData = GetBadGuysData(areaPointer);

    loadMapProperties(world);

    TileDataIterator = 0x03u;
    BadGuysDataIterator = 0x00u;

    CurrentPage = 0x00u;

    TileSkipToPage = 0x00u;
    BadGuysSkipToPage = 0x00u;

    CurrentMazeScore = 0x00u;

    LastTileObjectPageFlag = false;
    LastBadGuysObjectPageFlag = false;
    LastBadGuysObjectWasPageSkip = false;
    LastBadGuysObjectWasBalanceLift = false;

    HoleBufferLength = 0x00u;

    BackgroundModifiers = getBackgroundodifiers();
    setTheme(world.CurrentTheme);

    extendMap(world);

    for (uint8_t i = 0u; i < 3u; ++i) {
        loadPage(world);
    }
}

void MapLoader::parseTileObject(const uint8_t& b1, const uint8_t& b2, World& world, const unsigned int& pageColumnOffset) {
    const uint8_t xPos = ObjectXPos(b1, b2);
    const uint8_t yPos = ObjectYPos(b1, b2);
    const uint8_t category = ObjectCategory(b1, b2);
    const uint8_t metadata = ObjectMetadata(b1, b2);
    const uint8_t length = ObjectLength(b1, b2);

    const uint8_t basicBrick = getBasicBrick(world.CurrentTheme);
    const uint8_t basicBlock = getBasicBlock(world.CurrentTheme);

    const unsigned int colIndex = (pageColumnOffset + xPos) * 13u;
    const unsigned int tileIndex = colIndex + yPos;

    if (category == 0x00u) {
        if (yPos == 0x0Cu) {
            placeHole(length, tileIndex, world);
        }

        else if (yPos == 0x0Fu) {
            placeLiftsVerticalRope(colIndex, world);
        }

        else if (metadata == 0x00u) {
            world.m_Tiles[tileIndex] = std::make_unique<Blocks::RenderableCollideableItem>(gbl::TextureId::Block::Question, gbl::ItemType::SuperMushroom);
            world.m_AttributeTable[tileIndex] = 3u;
        }

        else if (metadata == 0x01u) {
            world.m_Tiles[tileIndex] = std::make_unique<Blocks::RenderableCollideableItem>(gbl::TextureId::Block::Question, gbl::ItemType::Coin);
            world.m_AttributeTable[tileIndex] = 3u;
        }

        else if (metadata == 0x02u) {
            world.m_Tiles[tileIndex] = std::make_unique<Blocks::Hidden>(gbl::ItemType::Coin);
            world.m_AttributeTable[tileIndex] = 1u;
        }

        else if (metadata == 0x03u) {
            if (world.m_SpawnOneUp || player.Data.Level != 0x01u) {
                world.m_Tiles[tileIndex] = std::make_unique<Blocks::Hidden>(gbl::ItemType::OneUp);
                world.m_AttributeTable[tileIndex] = 1u;

                world.m_SpawnOneUp = false;
            }
        }

        else if (metadata == 0x04u) {
            world.m_Tiles[tileIndex] = std::make_unique<Blocks::RenderableCollideableItem>(basicBrick, gbl::ItemType::SuperMushroom);
            world.m_AttributeTable[tileIndex] = GetSubPalleteIndex(basicBrick);
        }

        else if (metadata == 0x05u) {
            world.m_Tiles[tileIndex] = std::make_unique<Blocks::RenderableCollideableItem>(basicBrick, gbl::ItemType::Vine);
            world.m_AttributeTable[tileIndex] = GetSubPalleteIndex(basicBrick);
        }

        else if (metadata == 0x06u) {
            world.m_Tiles[tileIndex] = std::make_unique<Blocks::RenderableCollideableItem>(basicBrick, gbl::ItemType::Starman);
            world.m_AttributeTable[tileIndex] = GetSubPalleteIndex(basicBrick);
        }

        else if (metadata == 0x07u) {
            world.m_Tiles[tileIndex] = std::make_unique<Blocks::RenderableCollideableItem>(basicBrick, gbl::ItemType::Coin);
            world.m_AttributeTable[tileIndex] = GetSubPalleteIndex(basicBrick);
        }

        else if (metadata == 0x08u) {
            world.m_Tiles[tileIndex] = std::make_unique<Blocks::RenderableCollideableItem>(basicBrick, gbl::ItemType::OneUp);
            world.m_AttributeTable[tileIndex] = GetSubPalleteIndex(basicBrick);
        }

        else if (metadata == 0x09u) {
            placeSidewaysPipe(tileIndex, world);
        }

        else if (metadata == 0x0Au) {
            world.m_Tiles[tileIndex] = std::make_unique<Blocks::RenderableCollideable>(gbl::TextureId::Block::Question_used);
            world.m_AttributeTable[tileIndex] = 3u;
        }

        else if (metadata == 0x0Bu) {
            placeJumpSpring(tileIndex, xPos, pageColumnOffset, world);
        }

        else if (metadata == 0x0Cu) {
            // placeReverseLPipe();
        }
    }

    else if (category == 0x01u) {
        if (yPos == 0x0Cu) {
            // Balance's Horizontal Rope
        }

        else if (yPos == 0x0Fu) {
            // Vertical rope of balance lift
        }

        else {
            placeIsland(yPos, length, tileIndex, colIndex, MapType, world);
        }
    }

    else if (category == 0x02u) {
        if (yPos == 0x0Cu) {
            placeBridge(7u, length, colIndex, world);
        }

        else if (yPos == 0x0Fu) {
            placeCastle(length, colIndex, xPos, pageColumnOffset, world);
        }

        else {
            // Horizontal Bricks
            placeHorizontalRow<Blocks::RenderableCollideableBreakable>(tileIndex, length, GetSubPalleteIndex(basicBrick), world, basicBrick);
        }
    }

    else if (category == 0x03u) {
        if (yPos == 0x0Cu) {
            placeBridge(8u, length, colIndex, world);
        }

        else if (yPos == 0x0Fu) {
            placeStaircase(length, metadata, colIndex, world);
        }

        else {
            // Horizontal Blocks
            placeHorizontalRow<Blocks::RenderableCollideable>(tileIndex, length, GetSubPalleteIndex(basicBlock), world, basicBlock);
        }
    }

    else if (category == 0x04u) {
        if (yPos == 0x0Cu) {
            placeBridge(10, length, colIndex, world);
        }

        else if (yPos == 0x0Du) {
            if (metadata == 0x01u) {
                placeFlagpole(colIndex, xPos, pageColumnOffset, world);
            }

            else if (metadata == 0x02u) {
                spawnSprite(std::make_unique<Axe>(sf::Vector2f(pageColumnOffset + xPos, 7u) * TileSize), world);
            }

            else if (metadata == 0x03u) {
                placeBlock(colIndex + 7u, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::Chain), 0u, world);
            }

            else if (metadata == 0x04u) {
                placeHorizontalRow<Blocks::RenderableCollideable>(colIndex + 8u, 13u, 2u, world, gbl::TextureId::Bridge_bowser);
            }

            else if (metadata == 0x05u) {
                // Scroll Stop (warp zone)
            }

            else if (metadata == 0x07u) {
                // Scroll Stop
            }

            else if (metadata == 0x08u) {
                // Cheep Cheep
            }

            else {
                // Stop Continuation
            }
        }

        else if (yPos == 0x0Fu) {
            placeLongReverseLPipe(colIndex, world);
        }

        else {
            // Horizontal Coins
            placeHorizontalRow<Blocks::Coin>(tileIndex, length, 3u, world);
        }
    }

    else if (category == 0x05u) {
        if (yPos == 0x0Cu) {
            placeHoleFilledWithWater(length, tileIndex, world);
        }

        else if (yPos == 0x0Fu) {
            // Vertical Balls
            placeBlock(tileIndex, std::make_unique<Blocks::Flag>(gbl::TextureId::Block::Flag_top), 0u, world);
        }

        else if (yPos != 0x0Du) {
            // Vertical Bricks
            placeVerticalRow<Blocks::RenderableCollideableBreakable>(tileIndex, length, GetSubPalleteIndex(basicBrick), world, basicBrick);
        }
    }

    else if (category == 0x06u) {
        if (yPos == 0x0Cu) {
            placeHorizontalQuestionBlocksCoin(3u, length, colIndex, world);
        }

        else if (yPos != 0x0Du && yPos != 0x0Fu) {
            // Vertical Blocks
            placeVerticalRow<Blocks::RenderableCollideable>(tileIndex, length, GetSubPalleteIndex(basicBlock), world, basicBlock);
        }
    }

    else if (category == 0x07u) {
        if (yPos == 0x0Cu) {
            placeHorizontalQuestionBlocksCoin(7u, length, colIndex, world);
        }

        else {
            placePipe(length, metadata & 0x08u, tileIndex, xPos, yPos, pageColumnOffset, world);
        }
    }
}

void MapLoader::parseBadGuysObject(const uint8_t& b1, const uint8_t& b2, const bool& globalDifficulty, World& world, const unsigned int& pageColumnOffset) {
    const uint8_t xPos = ObjectXPos(b1, b2);
    const uint8_t yPos = ObjectYPos(b1, b2);
    const uint8_t category = EnemyCategory(b1, b2);

    const float x = (pageColumnOffset + static_cast<int8_t>(xPos) - 3) * 16.f;
    const sf::Vector2f position = sf::Vector2f((pageColumnOffset + xPos), yPos) * 16.f;

    if (category == 0x00u) {
        spawnSprite(std::make_unique<KoopaTroopa>(position), world);
    }

    else if (category == 0x02u || (World::Difficulty && category == 0x06u)) {
        spawnSprite(std::make_unique<BuzzyBeetle>(position), world);
    }

    else if (category == 0x03u) {
        spawnSprite(std::make_unique<RedKoopaTroopa>(position), world);
    }

    else if (category == 0x05u) {
        spawnSprite(std::make_unique<HammerBrother>(position), world);
    }

    else if (category == 0x06u) {
        spawnSprite(std::make_unique<Goomba>(position), world);
    }

    else if (category == 0x07u) {
        spawnSprite(std::make_unique<Bloober>(position), world);
    }

    else if (category == 0x0Eu) {
        spawnSprite(std::make_unique<KoopaParatroopa>(position), world);
    }

    else if (category == 0x0Fu) {
        spawnSprite(std::make_unique<RedKoopaParatroopa>(position), world);
    }

    else if (category == 0x1Bu) {
        spawnSprite(std::make_unique<Firebar>(position, false, gbl::Direction::Right, false), world);
    }

    else if (category == 0x1Cu) {
        spawnSprite(std::make_unique<Firebar>(position, false, gbl::Direction::Right, true), world);
    }

    else if (category == 0x1Du) {
        spawnSprite(std::make_unique<Firebar>(position, false, gbl::Direction::Left, false), world);
    }

    else if (category == 0x1Fu) {
        spawnSprite(std::make_unique<Firebar>(position, true, gbl::Direction::Right, false), world);
    }

    else if (category == 0x37u) {
        if (World::Difficulty) {
            spawnEnemyGroup<BuzzyBeetle>(x, 10u, 2u, world);
        } else {
            spawnEnemyGroup<Goomba>(x, 10u, 2u, world);
        }
    }

    else if (category == 0x38u) {
        if (World::Difficulty) {
            spawnEnemyGroup<BuzzyBeetle>(x, 10u, 3u, world);
        } else {
            spawnEnemyGroup<Goomba>(x, 10u, 3u, world);
        }
    }

    else if (category == 0x39u) {
        if (World::Difficulty) {
            spawnEnemyGroup<BuzzyBeetle>(x, 6u, 2u, world);
        } else {
            spawnEnemyGroup<Goomba>(x, 6u, 2u, world);
        }
    }

    else if (category == 0x3Au) {
        if (World::Difficulty) {
            spawnEnemyGroup<BuzzyBeetle>(x, 6u, 3u, world);
        } else {
            spawnEnemyGroup<Goomba>(x, 6u, 3u, world);
        }
    }

    else if (category == 0x3Bu) {
        spawnEnemyGroup<KoopaTroopa>(x, 10u, 2u, world);
    }

    else if (category == 0x3Cu) {
        spawnEnemyGroup<KoopaTroopa>(x, 10u, 3u, world);
    }

    else if (category == 0x25u) {
        spawnSprite(std::make_unique<LiftOscilating>(position, !globalDifficulty, true), world);
    }

    else if (category == 0x28u) {
        spawnSprite(std::make_unique<LiftOscilating>(position, !globalDifficulty, false), world);
    }

    else if (category == 0x27u) {
        spawnSprite(std::make_unique<LiftConstant>(position, !globalDifficulty, true), world);
    }

    else if (category == 0x26u) {
        spawnSprite(std::make_unique<LiftConstant>(position, !globalDifficulty, false), world);
    }

    else if (category == 0x29u) {
        spawnSprite(std::make_unique<LiftFall>(position, !globalDifficulty), world);
    }

    else if (category == 0x24u) {
        spawnSprite(std::make_unique<LiftBalance>(position), world);

        const unsigned int colIndex = (pageColumnOffset + xPos) * 13u;

        uint8_t anchorId = LastBadGuysObjectWasBalanceLift ? gbl::TextureId::Block::Anchor_2 : gbl::TextureId::Block::Anchor_1;
        placeBlock(colIndex + 13u, std::make_unique<Blocks::Renderable>(anchorId), 1u, world);

        placeVerticalRow<Blocks::Renderable>(colIndex + 14u, yPos - 3, 1u, world, gbl::TextureId::Block::String_2);

        if (LastBadGuysObjectWasBalanceLift) {
            unsigned int index = colIndex;

            while (true) {
                placeBlock(index, std::make_unique<Blocks::Renderable>(gbl::TextureId::Block::String_1), 1u, world);

                index -= 13;

                if (
                    Components::Render* renderComp = GetComponent(world.m_Tiles[index].get(), Components::Render);
                    renderComp && renderComp->TextureId == gbl::TextureId::Block::Anchor_1
                ) {
                    break;
                }
            }
        }

        LastBadGuysObjectWasBalanceLift ^= true;
    }
}

void MapLoader::loadPage(World& world) {
    const unsigned int pageOffset = (world.m_Tiles.size() - 208) / 13u;

    // create new page
    extendMap(world);

    // load background terrain and scenery

    for (uint8_t x = 0u; x < 16u; ++x) {
        const unsigned int colIndex = (pageOffset + x) * 13u;

        if (CurrentSceneryType) {
            // place scenery
            for (uint8_t y = 0u; y < 11u; ++y) {
                unsigned int tileIndex = colIndex + y;

                if (const uint8_t tile = getbackgroundTile(x, y, CurrentPage, CurrentSceneryType)) {
                    placeBlockIfEmpty(tileIndex, std::make_unique<Blocks::Renderable>(tile), GetSubPalleteIndex(tile, y), world);
                }
            }
        }

        if (CurrentTerrainType) {
            // place terrain
            uint8_t cellingLength = 0x00u;
            uint8_t middleLength = 0x00u;
            uint8_t surfaceLength = 0x00u;
            uint8_t terrainBlock_1;
            uint8_t terrainBlock_2;

            getTerrainData(cellingLength, middleLength, surfaceLength, terrainBlock_1, terrainBlock_2, world.CurrentTheme, CurrentTerrainType);

            if (cellingLength) {
                handleCellingTerrain(cellingLength, terrainBlock_1, colIndex, world);
            }

            if (middleLength) {
                handleMiddleTerrain(middleLength, terrainBlock_1, colIndex, world);
            }

            if (surfaceLength) {
                handleSurfaceTerrain(surfaceLength, terrainBlock_1, terrainBlock_2, colIndex, world);
            }
        }

        if (CurrentBackgroundType) {
            if (CurrentBackgroundType == 0x01u) {
                handleInWaterBackground(colIndex, world);
            }

            else if (CurrentBackgroundType == 0x02u) {
                handleWallBackground(colIndex, world);
            }

            else if (CurrentBackgroundType == 0x03u) {
                handleOverWaterBackground(colIndex, world);
            }
        }

        if (!BackgroundModifiers.empty() && BackgroundModifiers.front().first == (CurrentPage * 16u + x) * 13u) {
            const SceneData& sceneData = BackgroundModifiers.front().second;

            if (sceneData.Terrain >= 0x00) {
                CurrentTerrainType = sceneData.Terrain;
            }

            if (sceneData.Scenery >= 0x00) {
                CurrentSceneryType = sceneData.Scenery;
            }

            if (sceneData.background >= 0x00) {
                CurrentBackgroundType = sceneData.background;
            }

            BackgroundModifiers.erase(BackgroundModifiers.begin());
        }

    }

    if (HoleBufferLength) {
        placeHole(HoleBufferLength, pageOffset * 13u + 12u, world);
        HoleBufferLength = 0x00u;
    }

    if (CurrentPage >= TileSkipToPage) {
        // load map objects
        while (true) {
            const uint8_t& b1 = TileData[TileDataIterator];

            if (b1 == 0xFDu) {
                break; // end of tile structure data
            }

            const uint8_t& b2 = TileData[(++TileDataIterator)++];

            if (ObjectPageFlag(b1, b2)) {
                LastTileObjectPageFlag ^= 0x01u;

                if (LastTileObjectPageFlag) {
                    TileDataIterator -= 2;
                    break;
                }
            } else if (TileSkipToPage = ObjectIsPageSkip(b1, b2)) {
                break;
            }

            if (ObjectYPos(b1, b2) != 0x0Eu) {
                parseTileObject(b1, b2, world, pageOffset);
            }
        }
    }

    if (CurrentPage >= BadGuysSkipToPage) {
        const bool globalDifficulty = World::Difficulty || player.Data.GetLevelPointer() >= 18u;

        // load bad guys
        while (true) {
            const uint8_t& b1 = BadGuysData[BadGuysDataIterator];

            if (b1 == 0xFFu) {
                break; // end of bad guys data
            }

            const uint8_t& b2 = BadGuysData[(++BadGuysDataIterator)++];

            if (!LastBadGuysObjectWasPageSkip && ObjectPageFlag(b1, b2)) {
                LastBadGuysObjectPageFlag ^= 0x01u;

                if (LastBadGuysObjectPageFlag) {
                    BadGuysDataIterator -= 2;
                    break;
                }
            }

            const uint8_t yPos = ObjectYPos(b1, b2);

            const bool pageSkipObject = yPos == 0x0Fu;

            LastBadGuysObjectWasPageSkip = pageSkipObject;

            if (pageSkipObject) {
                BadGuysSkipToPage = b2;
                break;
            }

            if (yPos == 0x0Eu) {
                // room change
                /*const uint8_t& b3 =*/ BadGuysData[BadGuysDataIterator++];

                // const uint8_t newWorld = ((b3 >> 0x04u) / 2u) + 1u;
                // const uint8_t newPage = b3 & 0x0Fu;
            } else if (globalDifficulty || (b2 & 0x40u) == 0x00u) {
                parseBadGuysObject(b1, b2, globalDifficulty, world, pageOffset);
            }
        }
    }

    ++CurrentPage;
}

void MapLoader::Update(World& world) {
    constexpr float Factor = 256.f;

    if (world.CameraPosition >= (Factor + gbl::Width * 0.5f)) {
        world.CameraPosition -= Factor;
        player.Position.x -= Factor;

        const auto MoveSprite = [](Sprite* sprite) -> void {
            sprite->Position.x -= Factor;

            if (
                LiftOscilating* lift = GetIf(sprite, LiftOscilating);
                lift && !lift->m_Axis
            ) {
                lift->m_Start -= Factor;
            }
        };

        for (auto& sprite : world.m_Sprites) {
            if (sprite) {
                MoveSprite(sprite.get());
            }
        }

        for (auto& group : world.m_SpritePool) {
            for (auto& sprite : group) {
                if (sprite) {
                    MoveSprite(sprite.get());
                }
            }
        }

        for (auto& ball : world.m_Fireballs) {
            if (ball) {
                MoveSprite(ball.get());
            }
        }

        if (world.m_BouncingBlock) {
            world.m_BouncingBlock->Position.x -= Factor;
        }

        for (auto& sprite : world.m_MiscSprites) {
            if (sprite) {
                sprite->Position.x -= Factor;
            }
        }

        for (auto& animation : world.m_DeathAnimations) {
            if (animation) {
                animation->Position.x -= Factor;
            }
        }

        world.m_Tiles.erase(world.m_Tiles.begin(), world.m_Tiles.begin() + 208u);
        world.m_AttributeTable.erase(world.m_AttributeTable.begin(), world.m_AttributeTable.begin() + 208u);

        handleMaze(world);

        loadPage(world);
    }
}