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
        MarauderSpawn,
        AssaultSpawn,
        ShieldSpawn,
        HeavySpawn,
        RadioSpawn,
        BossSpawn,
        Entrance,
        Exit
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
        {"PlayerSpawn", TileType::PlayerSpawn},
        {"Entrance", TileType::Entrance},
        {"Exit", TileType::Exit}
    };

    struct ItemPlacement
    {
        int column = 0;
        int row = 0;
        std::string itemId;
    };

    struct BossSpec
    {
        std::string bossId;
        float healthScale = 1.f;
        float damageScale = 1.f;

        bool IsEmpty() const { return bossId.empty(); }
    };

    struct LevelInfo
    {
        std::string title;
        std::string nextLevelId;
        BossSpec boss;
    };

    struct LevelData
    {
        int width = 0;
        int height = 0;
        std::vector<std::vector<TileType>> tiles;
        std::vector<ItemPlacement> items;
        LevelInfo info;
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
