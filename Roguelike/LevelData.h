#pragma once

#include <string>
#include <vector>

namespace RoguelikeGame
{
    enum class TileType
    {
        Empty,
        Floor,
        Wall,
        PlayerSpawn,
        GruntSpawn,
        AssaultSpawn,
        ShieldSpawn,
        HeavySpawn,
        RadioSpawn,
        BossSpawn
    };

    struct TileTypeName
    {
        const char* name;
        TileType type;
    };

    constexpr TileTypeName TILE_TYPE_NAMES[] = {
        {"Empty", TileType::Empty},
        {"Floor", TileType::Floor},
        {"Wall", TileType::Wall},
        {"PlayerSpawn", TileType::PlayerSpawn}
    };

    struct ItemPlacement
    {
        int column = 0;
        int row = 0;
        std::string itemId;
    };

    struct LevelData
    {
        int width = 0;
        int height = 0;
        std::vector<std::vector<TileType>> tiles;
        std::vector<ItemPlacement> items;
    };

    inline int CountTiles(const LevelData& levelData, TileType tileType)
    {
        int count = 0;
        for (const auto& row : levelData.tiles)
        {
            for (TileType tile : row)
            {
                if (tile == tileType)
                {
                    count++;
                }
            }
        }

        return count;
    }
}
