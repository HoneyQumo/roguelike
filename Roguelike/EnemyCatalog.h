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
             50.f, 30.f, 0.9f, 0.f, "grunt"}
        },
        {
            TileType::MarauderSpawn, 'm', "MarauderSpawn",
            {"Marauder", "enemy_assault", WeaponId::Glock,
             120.f, 160.f, 140.f, 45.f, 0.f,
             240.f, 10.f, 0.8f, 620.f, "marauder"}
        },
        {
            TileType::AssaultSpawn, 'a', "AssaultSpawn",
            {"Assault", "enemy_assault", WeaponId::Ak47,
             110.f, 420.f, 220.f, 70.f, 5.f,
             360.f, 12.f, 1.4f, 700.f, "assault"}
        },
        {
            TileType::ShieldSpawn, 's', "ShieldSpawn",
            {"Shield", "enemy_shield", WeaponId::Glock,
             95.f, 380.f, 150.f, 120.f, 14.f,
             300.f, 9.f, 1.1f, 650.f, "shield"}
        },
        {
            TileType::HeavySpawn, 'h', "HeavySpawn",
            {"Heavy", "enemy_heavy", WeaponId::M16,
             80.f, 400.f, 200.f, 150.f, 10.f,
             340.f, 6.f, 0.3f, 720.f, "heavy"}
        },
        {
            TileType::RadioSpawn, 'r', "RadioSpawn",
            {"Radio", "enemy_radio", WeaponId::SmgSuppressed,
             165.f, 460.f, 260.f, 55.f, 2.f,
             300.f, 5.f, 0.45f, 680.f, "radio"}
        },
        {
            TileType::BossSpawn, 'b', "BossSpawn",
            {"Boss", "enemy_boss", WeaponId::ShotgunPump,
             90.f, 500.f, 170.f, 260.f, 18.f,
             260.f, 16.f, 1.0f, 900.f, "boss"}
        }
    };

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
