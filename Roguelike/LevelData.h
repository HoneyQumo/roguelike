#pragma once

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
        {"PlayerSpawn", TileType::PlayerSpawn},
        {"GruntSpawn", TileType::GruntSpawn},
        {"AssaultSpawn", TileType::AssaultSpawn},
        {"ShieldSpawn", TileType::ShieldSpawn},
        {"HeavySpawn", TileType::HeavySpawn},
        {"RadioSpawn", TileType::RadioSpawn},
        {"BossSpawn", TileType::BossSpawn}
    };

    struct LevelData
    {
        int width = 0;
        int height = 0;
        std::vector<std::vector<TileType>> tiles;
    };
}
