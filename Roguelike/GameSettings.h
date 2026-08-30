#pragma once

#include <SFML/Graphics/Color.hpp>
#include "EnemyConfig.h"
#include "SpriteAtlas.h"
#include "WeaponCatalog.h"

namespace RoguelikeGame
{
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;

    constexpr float TILE_SIZE = 64.f;

    constexpr int CHARACTER_SPRITE_SIZE = CHARACTER_FRAME_SIZE;

    constexpr float CHARACTER_COLLIDER_SIZE = 30.f;

    constexpr float PLAYER_SPEED = 250.f;
    constexpr float PLAYER_MAX_HEALTH = 100.f;
    constexpr float PLAYER_ARMOR = 5.f;
    constexpr float PLAYER_ATTACK_DAMAGE = 25.f;
    constexpr float PLAYER_ATTACK_COOLDOWN = 0.3f;
    constexpr float PLAYER_PROJECTILE_SPEED = 800.f;
    constexpr auto PLAYER_WEAPON = WeaponId::Ak47;

    constexpr float PROJECTILE_COLLIDER_SIZE = 8.f;
    constexpr float PROJECTILE_LIFETIME = 2.f;

    constexpr float SHOT_FORWARD_OFFSET = 0.5f * CHARACTER_COLLIDER_SIZE + 4.f;

    inline Vector2Df ShotOffset(const WeaponDefinition& weapon)
    {
        return {SHOT_FORWARD_OFFSET, ToWorldOffset(weapon.muzzleX, weapon.muzzleY).y};
    }

    constexpr int CROSSHAIR_SIZE = 32;

    constexpr float HEALTH_BAR_WIDTH = 48.f;
    constexpr float HEALTH_BAR_HEIGHT = 6.f;
    constexpr float HEALTH_BAR_OFFSET_Y = 30.f;

    constexpr float HIT_FLASH_DURATION = 0.12f;
    constexpr auto HIT_FLASH_UNIFORM = "amount";

    // Лужа стартует на третьем кадре анимации смерти и продолжает расти после того, как тело замерло.
    constexpr float BLOOD_POOL_DELAY = 3.f * 110.f / 1000.f;

    constexpr float MUSIC_VOLUME = 15.f;
    constexpr float SHOT_VOLUME = 20.f;
    constexpr float HURT_VOLUME = 35.f;

    constexpr auto PLAYER_TEXTURE = "player";
    constexpr auto WEAPONS_TEXTURE = "weapons";
    constexpr auto CROSSHAIR_TEXTURE = "crosshair";
    constexpr auto MUZZLE_FLASH_TEXTURE = "fx_muzzle_flash";
    constexpr auto BLOOD_POOL_TEXTURE = "fx_blood_pool";
    constexpr auto BLOOD_HIT_TEXTURE = "fx_blood_hit";
    constexpr auto IMPACT_TEXTURE = "fx_impact";
    constexpr auto BULLET_TEXTURE = "fx_bullet";
    constexpr auto HIT_FLASH_SHADER = "hit_flash";

    constexpr auto TEST_LEVEL_PATH = "Resources/Levels/test_level.config";
    constexpr auto LOG_FILE_PATH = "log.txt";

    const sf::Color WALL_COLOR = {92, 86, 80};
    const sf::Color FLOOR_COLOR = {46, 42, 38};
    const sf::Color CROSSHAIR_COLOR = {255, 255, 255};

    const EnemyConfig GRUNT_CONFIG = {
        "Grunt", "enemy_grunt", WeaponId::Knife,
        150.f, 300.f, 40.f, 50.f, 0.f,
        0.f, 0.f, 0.f, 0.f
    };

    const EnemyConfig ASSAULT_CONFIG = {
        "Assault", "enemy_assault", WeaponId::Ak47,
        110.f, 420.f, 220.f, 70.f, 5.f,
        360.f, 12.f, 1.4f, 700.f
    };

    const EnemyConfig SHIELD_CONFIG = {
        "Shield", "enemy_shield", WeaponId::Glock,
        95.f, 380.f, 150.f, 120.f, 14.f,
        300.f, 9.f, 1.1f, 650.f
    };

    const EnemyConfig HEAVY_CONFIG = {
        "Heavy", "enemy_heavy", WeaponId::M16,
        80.f, 400.f, 200.f, 150.f, 10.f,
        340.f, 6.f, 0.3f, 720.f
    };

    const EnemyConfig RADIO_CONFIG = {
        "Radio", "enemy_radio", WeaponId::SmgSuppressed,
        165.f, 460.f, 260.f, 55.f, 2.f,
        300.f, 5.f, 0.45f, 680.f
    };

    const EnemyConfig BOSS_CONFIG = {
        "Boss", "enemy_boss", WeaponId::ShotgunPump,
        90.f, 500.f, 170.f, 260.f, 18.f,
        260.f, 26.f, 1.6f, 900.f
    };
}
