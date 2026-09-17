#pragma once

#include <string>
#include <vector>
#include "FightStyle.h"

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
        Exit,
        Door,
        Line,
        Water,
        WaveSpawn,
        Breach
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
        {"Exit", TileType::Exit},
        {"Line", TileType::Line},
        {"Water", TileType::Water},
        {"WaveSpawn", TileType::WaveSpawn},
        {"Breach", TileType::Breach}
    };

    struct PropPlacement
    {
        int column = 0;
        int row = 0;
        std::string propId;
        float angle = 0.f;
    };

    struct ItemPlacement
    {
        int column = 0;
        int row = 0;
        std::string itemId;
    };

    struct DoorPlacement
    {
        int column = 0;
        int row = 0;
        std::string doorId;
    };

    struct FixturePlacement
    {
        int column = 0;
        int row = 0;
        std::string id;
    };

    struct WaveEntry
    {
        TileType enemy = TileType::Empty;
        int count = 0;
    };

    struct WaveSpec
    {
        float delay = 0.f;
        std::vector<WaveEntry> entries;

        int Size() const
        {
            int total = 0;
            for (const WaveEntry& entry : entries)
            {
                total += entry.count;
            }

            return total;
        }
    };

    /**
    *	Погоня вместо волн: за игроком постоянно висит несколько преследователей,
    *	убитого заменяет следующий, и кончается это только побегом.
    *	Кто именно выбегает, зависит от того, сколько пути позади.
    */
    struct PursuitEntry
    {
        TileType enemy = TileType::Empty;
        int weight = 0;
    };

    struct PursuitEchelon
    {
        // С какой доли пути этот состав выходит на сцену.
        float fromPart = 0.f;
        std::vector<PursuitEntry> entries;

        int TotalWeight() const
        {
            int total = 0;
            for (const PursuitEntry& entry : entries)
            {
                total += entry.weight;
            }

            return total;
        }
    };

    struct PursuitSpec
    {
        int keep = 0;
        int grow = 0;
        float respawn = 0.f;
        std::vector<PursuitEchelon> echelons;

        // Погоня должна давить, а не отступать и пережидать за укрытием.
        FightStyle style = RELENTLESS_FIGHT;

        bool IsEmpty() const { return keep <= 0 || echelons.empty(); }
    };

    struct ZonePlacement
    {
        int column = 0;
        int row = 0;
        std::string zoneId;
    };

    struct PatrolPoint
    {
        int column = 0;
        int row = 0;
        std::string routeId;
        int order = 0;
        bool isWatch = false;
    };

    struct BossSpec
    {
        std::string bossId;
        float healthScale = 1.f;
        float damageScale = 1.f;
        std::string drop;

        bool IsEmpty() const { return bossId.empty(); }
    };

    struct LevelInfo
    {
        std::string title;
        std::string nextLevelId;
        std::string kind;
        std::string tileset;
        std::string music;
        std::string ambient;
        int fogRadius = 0;
        FightStyle style;
        BossSpec boss;
    };

    using TileGrid = std::vector<std::vector<TileType>>;

    inline TileType GridAt(const TileGrid& grid, int column, int row)
    {
        if (row < 0 || row >= static_cast<int>(grid.size()))
        {
            return TileType::Empty;
        }

        const std::vector<TileType>& line = grid[row];

        return column < 0 || column >= static_cast<int>(line.size()) ? TileType::Empty : line[column];
    }

    struct LevelData
    {
        int width = 0;
        int height = 0;
        TileGrid tiles;

        // Верхний слой: только рисунок. Проходимость и коллизии решает нижний,
        // иначе пришлось бы разбирать, что делать, когда слои спорят.
        TileGrid overlay;
        std::vector<ItemPlacement> items;
        std::vector<PropPlacement> props;
        std::vector<PatrolPoint> patrols;
        std::vector<DoorPlacement> doors;
        std::vector<ZonePlacement> zones;
        std::vector<WaveSpec> waves;
        FightStyle wavesStyle;
        PursuitSpec pursuit;
        std::vector<FixturePlacement> levers;
        std::vector<FixturePlacement> hatches;
        std::vector<FixturePlacement> escapes;
        LevelInfo info;
    };

    inline TileType TileAt(const LevelData& levelData, int column, int row)
    {
        return GridAt(levelData.tiles, column, row);
    }

    inline TileType OverlayAt(const LevelData& levelData, int column, int row)
    {
        return GridAt(levelData.overlay, column, row);
    }

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
