#pragma once

#include "EnemyConfig.h"
#include "LevelData.h"

namespace RoguelikeGame
{
    struct EnemyDefinition
    {
        TileType tile;
        char levelSymbol;
        const char* tileName;
        EnemyConfig config;
    };

    inline constexpr EnemyDefinition ENEMIES[] = {
        {
            TileType::GruntSpawn, 'g', "GruntSpawn",
            {"Grunt", "enemy_grunt", WeaponId::Knife,
             150.f, 300.f, 40.f, 50.f, 0.f,
             50.f, 30.f, 0.9f, 0.f, "grunt", 5.f, 45.f, 70.f, 3.0f, 2.2f, 40.f, 8, 1, 2, 2.0f, 1.30f, 0.9f, 0.7f, "heavy", 3, 90.f, SHOUT_RADIUS, 1.0f, 250.f}
        },
        {
            TileType::MarauderSpawn, 'm', "MarauderSpawn",
            {"Marauder", "enemy_assault", WeaponId::Glock,
             120.f, 160.f, 140.f, 45.f, 0.f,
             240.f, 7.5f, 0.40f, 620.f, "marauder", 5.f, 50.f, 75.f, 3.5f, 2.4f, 45.f, 8, 1, 2, 2.2f, 1.30f, 1.0f, 0.8f, "guard", 3, 95.f, SHOUT_RADIUS, 2.5f, 240.f}
        },
        {
            TileType::AssaultSpawn, 'a', "AssaultSpawn",
            {"Assault", "enemy_assault", WeaponId::Ak47,
             110.f, 420.f, 220.f, 70.f, 40.f,
             360.f, 9.f, 0.70f, 700.f, "assault", 5.f, 60.f, 85.f, 4.5f, 2.8f, 55.f, 10, 2, 3, 2.6f, 1.40f, 1.2f, 0.9f, "guard", 3, 105.f, SHOUT_RADIUS, 2.5f, 267.f}
        },
        {
            TileType::ShieldSpawn, 's', "ShieldSpawn",
            {"Shield", "enemy_shield", WeaponId::Glock,
             95.f, 380.f, 150.f, 120.f, 120.f,
             300.f, 6.f, 0.55f, 650.f, "shield", 5.f, 55.f, 80.f, 4.0f, 2.6f, 50.f, 10, 1, 3, 2.4f, 1.40f, 1.1f, 0.8f, "heavy", 3, 100.f, SHOUT_RADIUS, 2.5f, 210.f}
        },
        {
            TileType::HeavySpawn, 'h', "HeavySpawn",
            {"Heavy", "enemy_heavy", WeaponId::M16,
             80.f, 400.f, 200.f, 150.f, 90.f,
             340.f, 4.5f, 0.15f, 720.f, "heavy", 5.f, 65.f, 90.f, 5.0f, 3.0f, 60.f, 10, 2, 3, 2.8f, 1.45f, 1.0f, 0.7f, "heavy", 3, 110.f, SHOUT_RADIUS, 2.5f, 207.f}
        },
        {
            TileType::RadioSpawn, 'r', "RadioSpawn",
            {"Radio", "enemy_radio", WeaponId::SmgSuppressed,
             165.f, 460.f, 260.f, 55.f, 20.f,
             300.f, 3.7f, 0.22f, 680.f, "radio", 5.f, 75.f, 100.f, 5.5f, 4.0f, 80.f, 12, 2, 4, 3.2f, 1.60f, 1.8f, 1.2f, "radio", 3, 120.f, RADIO_SHOUT_RADIUS, 2.5f, 268.f}
        },
        {
            TileType::BossSpawn, 'b', "BossSpawn",
            {"Boss", "enemy_boss", WeaponId::ShotgunPump,
             90.f, 500.f, 170.f, 260.f, 150.f,
             260.f, 12.f, 0.70f, 900.f, "boss", 5.f, 90.f, 120.f, 8.0f, 4.5f, 90.f, 12, 2, 4, 3.5f, 1.60f, 1.6f, 1.1f, "radio", 3, 135.f, SHOUT_RADIUS, 2.0f, 0.f}
        }
    };

    constexpr const EnemyDefinition* FindEnemyBySymbol(char symbol)
    {
        for (const EnemyDefinition& enemy : ENEMIES)
        {
            if (enemy.levelSymbol == symbol)
            {
                return &enemy;
            }
        }

        return nullptr;
    }

    constexpr const EnemyDefinition* FindEnemy(TileType tile)
    {
        for (const EnemyDefinition& enemy : ENEMIES)
        {
            if (enemy.tile == tile)
            {
                return &enemy;
            }
        }

        return nullptr;
    }

    constexpr const EnemyConfig* FindEnemyConfig(TileType tile)
    {
        const EnemyDefinition* enemy = FindEnemy(tile);
        return enemy == nullptr ? nullptr : &enemy->config;
    }
}
