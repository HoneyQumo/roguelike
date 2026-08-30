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

    struct LevelData
    {
        int width = 0;
        int height = 0;
        std::vector<std::vector<TileType>> tiles;
    };
}
