#pragma once

#include <string>
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
    constexpr float PLAYER_RUN_SPEED_MULTIPLIER = 1.5f;
    constexpr float PLAYER_MAX_HEALTH = 100.f;
    constexpr float PLAYER_ARMOR = 5.f;
    constexpr float PLAYER_ATTACK_DAMAGE = 25.f;
    constexpr float PLAYER_ATTACK_COOLDOWN = 0.3f;
    constexpr float PLAYER_PROJECTILE_SPEED = 800.f;
    constexpr float PLAYER_MELEE_DAMAGE = 15.f;
    constexpr float PLAYER_HEAVY_LUNGE_SPEED = 150.f;

    constexpr int PLAYER_WEAPON_SLOTS = 3;
    constexpr int PLAYER_START_WEAPON_SLOT = 0;
    constexpr WeaponId PLAYER_LOADOUT[PLAYER_WEAPON_SLOTS] = {WeaponId::Ak47, WeaponId::Glock, WeaponId::Bat};

    struct AmmoReserve
    {
        AmmoKind kind;
        int count;
    };

    constexpr AmmoReserve PLAYER_START_AMMO[] = {
        {AmmoKind::Rifle, 300},
        {AmmoKind::Smg, 200},
        {AmmoKind::Pistol, 120},
        {AmmoKind::Shell, 48},
        {AmmoKind::Rocket, 6}
    };

    constexpr float ReloadFramesPerSecond(float reloadTime)
    {
        return reloadTime > 0.f ? RELOAD_ANIMATION.frames / reloadTime : RELOAD_ANIMATION.framesPerSecond;
    }

    constexpr float PROJECTILE_COLLIDER_SIZE = 8.f;
    constexpr float PROJECTILE_LIFETIME = 2.f;

    constexpr float SHOT_FORWARD_OFFSET = 0.5f * CHARACTER_COLLIDER_SIZE + 4.f;

    inline XYZEngine::Vector2Df ShotOffset(const WeaponDefinition& weapon)
    {
        return {SHOT_FORWARD_OFFSET, ToWorldOffset(weapon.muzzleX, weapon.muzzleY).y};
    }

    constexpr int GROUND_RENDER_LAYER = 0;
    constexpr int BLOOD_RENDER_LAYER = 1;
    constexpr int CORPSE_RENDER_LAYER = 2;
    constexpr int ENEMY_RENDER_LAYER = 3;
    constexpr int STOWED_WEAPON_RENDER_LAYER = 4;
    constexpr int PLAYER_RENDER_LAYER = 5;
    constexpr int EFFECT_RENDER_LAYER = 6;
    constexpr int UI_RENDER_LAYER = 7;

    constexpr int CROSSHAIR_SIZE = 32;
    constexpr int RELOAD_MAG_FRAME_SIZE = 64;
    constexpr int RELOAD_MAG_FRAMES = 30;
    constexpr int RELOAD_INDICATOR_SIZE = 44;

    constexpr int AMMO_HUD_FONT_SIZE = 30;
    constexpr int AMMO_HUD_NAME_FONT_SIZE = 18;
    constexpr float AMMO_HUD_MARGIN_X = 26.f;
    constexpr float AMMO_HUD_MARGIN_Y = 22.f;
    constexpr float AMMO_HUD_LINE_HEIGHT = 1.35f;
    constexpr float AMMO_HUD_OUTLINE = 2.f;

    // Красная зона обоймы
    constexpr float AMMO_HUD_LOW_PART = 0.25f;

    constexpr float HEALTH_BAR_WIDTH = 48.f;
    constexpr float HEALTH_BAR_HEIGHT = 6.f;
    constexpr float HEALTH_BAR_OFFSET_Y = 30.f;

    constexpr float HIT_FLASH_DURATION = 0.12f;
    constexpr auto HIT_FLASH_UNIFORM = "amount";

    // Лужа кров начинается с 3го кадра анимации смерти
    constexpr float BLOOD_POOL_DELAY = 3.f * 110.f / 1000.f;

    constexpr float MUSIC_VOLUME = 15.f;
    constexpr float SHOT_VOLUME = 20.f;
    constexpr float RELOAD_VOLUME = 45.f;
    constexpr float HURT_VOLUME = 35.f;
    constexpr float MELEE_HIT_VOLUME = 55.f;

    constexpr float HEAVY_CHARGED_GLOW = 0.22f;
    constexpr float HEAVY_CHARGED_GLOW_PERIOD = 0.18f;

    constexpr auto PLAYER_TEXTURE = "player";
    constexpr auto WEAPONS_TEXTURE = "weapons";
    constexpr auto CROSSHAIR_TEXTURE = "crosshair";
    constexpr auto MUZZLE_FLASH_TEXTURE = "fx_muzzle_flash";
    constexpr auto BLOOD_POOL_TEXTURE = "fx_blood_pool";
    constexpr auto BLOOD_HIT_TEXTURE = "fx_blood_hit";
    constexpr auto IMPACT_TEXTURE = "fx_impact";
    constexpr auto BULLET_TEXTURE = "fx_bullet";
    constexpr auto RELOAD_MAG_TEXTURE = "reload_mag";
    constexpr auto HIT_FLASH_SHADER = "hit_flash";
    constexpr auto HUD_FONT = "hud";
    constexpr auto SHOT_SOUND = "shot";
    constexpr auto HURT_SOUND = "hurt";
    constexpr auto MAIN_THEME_MUSIC = "main_theme";

    const std::string TEXTURES_PATH = "Resources/Textures/";
    const std::string AUDIO_PATH = "Resources/Audio/";
    const std::string SHADERS_PATH = "Resources/Shaders/";
    const std::string LEVELS_PATH = "Resources/Levels/";
    const std::string FONTS_PATH = "Resources/Fonts/";
    const std::string WEAPONS_AUDIO_PATH = "Resources/Audio/Weapons/";

    const std::string CROSSHAIR_FILE = TEXTURES_PATH + "crosshair.png";
    const std::string WEAPONS_ATLAS_FILE = TEXTURES_PATH + "weapons.png";
    const std::string RELOAD_MAG_FILE = TEXTURES_PATH + "reload_mag.png";
    const std::string FX_ATLAS_FILE = TEXTURES_PATH + "fx.png";
    const std::string HIT_FLASH_SHADER_FILE = SHADERS_PATH + "hit_flash.frag";
    const std::string SHOT_SOUND_FILE = AUDIO_PATH + "shot.wav";
    const std::string HURT_SOUND_FILE = AUDIO_PATH + "hurt.wav";
    const std::string MAIN_THEME_FILE = AUDIO_PATH + "main_music_1.ogg";
    const std::string TEST_LEVEL_FILE = LEVELS_PATH + "test_level.config";
    const std::string HUD_FONT_FILE = FONTS_PATH + "Roboto-Medium.ttf";

    constexpr auto LOG_FILE_PATH = "log.txt";

    const sf::Color WALL_COLOR = {92, 86, 80};
    const sf::Color FLOOR_COLOR = {46, 42, 38};
    const sf::Color CROSSHAIR_COLOR = {255, 255, 255};
    const sf::Color AMMO_HUD_COLOR = {235, 230, 220};
    const sf::Color AMMO_HUD_LOW_COLOR = {220, 90, 70};
    const sf::Color AMMO_HUD_RELOADING_COLOR = {235, 190, 90};
    const sf::Color AMMO_HUD_OUTLINE_COLOR = {15, 13, 12, 220};
    const sf::Color RELOAD_INDICATOR_COLOR = {235, 190, 90};

    const EnemyConfig GRUNT_CONFIG = {
        "Grunt", "enemy_grunt", WeaponId::Knife,
        150.f, 300.f, 40.f, 50.f, 0.f,
        50.f, 30.f, 0.9f, 0.f
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
