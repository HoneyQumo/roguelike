#pragma once

#include <string>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <CameraComponent.h>
#include "SpriteAtlas.h"
#include "WeaponCatalog.h"

namespace RoguelikeGame
{
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;
    constexpr unsigned int FRAME_RATE_LIMIT = 60;

    constexpr float CAMERA_VIEW_HEIGHT = static_cast<float>(SCREEN_HEIGHT);
    constexpr float CAMERA_SHAKE_LIMIT = 40.f;
    constexpr float HIT_STOP_LIGHT = 0.035f;
    constexpr float HIT_STOP_HEAVY = 0.07f;
    constexpr float DEATH_TIME_SCALE = 0.35f;
    constexpr float DEATH_SLOW_MOTION_TIME = 0.7f;
    constexpr float DEATH_SLOW_MOTION_BLEND = 0.6f;
    constexpr XYZEngine::CameraShake CAMERA_SHAKE_LIGHT = {5.f, 0.12f, 24.f};
    constexpr XYZEngine::CameraShake CAMERA_SHAKE_HEAVY = {13.f, 0.28f, 18.f};
    constexpr XYZEngine::CameraShake CAMERA_SHAKE_BLAST = {24.f, 0.45f, 14.f};

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

    constexpr float PLAYER_ROLL_SPEED = 900.f;
    constexpr float PLAYER_ROLL_MAX_STEP = 0.75f * CHARACTER_COLLIDER_SIZE;
    constexpr float PLAYER_ROLL_COOLDOWN = 0.35f;

    constexpr unsigned int PLAYER_COLLISION_LAYER = 1u << 1;
    constexpr unsigned int ENEMY_COLLISION_LAYER = 1u << 2;

    constexpr int PLAYER_WEAPON_SLOTS = 3;
    constexpr int PLAYER_START_WEAPON_SLOT = 0;
    constexpr WeaponId PLAYER_LOADOUT[PLAYER_WEAPON_SLOTS] = {WeaponId::Rpg, WeaponId::ShotgunPump, WeaponId::Bat};

    constexpr int NO_WEAPON_SLOT = -1;

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

    constexpr float ReloadFrameSeconds(float reloadTime)
    {
        return reloadTime > 0.f ? reloadTime / RELOAD_ANIMATION.frames : RELOAD_ANIMATION.secondsPerFrame;
    }

    constexpr float PROJECTILE_COLLIDER_SIZE = 8.f;
    constexpr float PROJECTILE_LIFETIME = 2.f;
    constexpr float ROCKET_SPRITE_SCALE = 2.f;
    constexpr float ROCKET_BODY_OFFSET = 12.f * ROCKET_SPRITE_SCALE;
    constexpr float EXPLOSION_CORE_RADIUS = 0.5f * TILE_SIZE;

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

    constexpr int OVERLAY_TITLE_FONT_SIZE = 56;
    constexpr int OVERLAY_HINT_FONT_SIZE = 24;
    constexpr float OVERLAY_LINE_GAP = 36.f;
    constexpr sf::Keyboard::Key RESTART_KEY = sf::Keyboard::R;
    constexpr float GAME_OVER_DELAY = 1.5f;
    constexpr auto PAUSE_TITLE = u8"ПАУЗА";
    constexpr auto PAUSE_HINT = u8"Esc — продолжить";
    constexpr auto GAME_OVER_TITLE = u8"ВЫ ПОГИБЛИ";
    constexpr auto GAME_OVER_HINT = u8"R — заново";

    constexpr float HEALTH_BAR_WIDTH = 48.f;
    constexpr float HEALTH_BAR_HEIGHT = 6.f;
    constexpr float HEALTH_BAR_OFFSET_Y = 30.f;

    constexpr float HIT_FLASH_DURATION = 0.12f;
    constexpr auto HIT_FLASH_UNIFORM = "amount";

    // Лужа крови начинается с 3го кадра анимации смерти
    constexpr float BLOOD_POOL_DELAY = 3.f * DEATH_ANIMATION.secondsPerFrame;

    constexpr float MUSIC_VOLUME = 15.f;
    constexpr float SHOT_VOLUME = 20.f;
    constexpr float RELOAD_VOLUME = 45.f;
    constexpr float HURT_VOLUME = 35.f;
    constexpr float MELEE_HIT_VOLUME = 55.f;

    constexpr float HEAVY_CHARGED_GLOW = 0.22f;
    constexpr float HEAVY_CHARGED_GLOW_PERIOD = 0.18f;

    constexpr auto PLAYER_OBJECT_NAME = "Player";

    constexpr auto PLAYER_TEXTURE = "player";
    constexpr auto WEAPONS_TEXTURE = "weapons";
    constexpr auto CROSSHAIR_TEXTURE = "crosshair";
    constexpr auto MUZZLE_FLASH_TEXTURE = "fx_muzzle_flash";
    constexpr auto BLOOD_POOL_TEXTURE = "fx_blood_pool";
    constexpr auto BLOOD_HIT_TEXTURE = "fx_blood_hit";
    constexpr auto IMPACT_TEXTURE = "fx_impact";
    constexpr auto BULLET_TEXTURE = "fx_bullet";
    constexpr auto ROCKET_TEXTURE = "fx_rocket";
    constexpr auto EXPLOSION_TEXTURE = "fx_explosion";
    constexpr auto RELOAD_MAG_TEXTURE = "reload_mag";
    constexpr auto HIT_FLASH_SHADER = "hit_flash";
    constexpr auto HUD_FONT = "hud";
    constexpr auto SHOT_SOUND = "shot";
    constexpr auto HURT_SOUND = "hurt";
    constexpr auto MAIN_THEME_MUSIC = "main_theme";

    constexpr auto TEXTURES_PATH = "Resources/Textures/";
    constexpr auto AUDIO_PATH = "Resources/Audio/";
    constexpr auto WEAPONS_AUDIO_PATH = "Resources/Audio/Weapons/";

    constexpr auto CROSSHAIR_FILE = "Resources/Textures/crosshair.png";
    constexpr auto WEAPONS_ATLAS_FILE = "Resources/Textures/weapons.png";
    constexpr auto RELOAD_MAG_FILE = "Resources/Textures/reload_mag.png";
    constexpr auto FX_ATLAS_FILE = "Resources/Textures/fx.png";
    constexpr auto HIT_FLASH_SHADER_FILE = "Resources/Shaders/hit_flash.frag";
    constexpr auto SHOT_SOUND_FILE = "Resources/Audio/shot.wav";
    constexpr auto HURT_SOUND_FILE = "Resources/Audio/hurt.wav";
    constexpr auto MAIN_THEME_FILE = "Resources/Audio/main_music_1.ogg";
    constexpr auto TEST_LEVEL_FILE = "Resources/Levels/test_level.config";
    constexpr auto HUD_FONT_FILE = "Resources/Fonts/Roboto-Medium.ttf";

    constexpr auto LOG_FILE_PATH = "log.txt";

    inline const sf::Color WALL_COLOR = {92, 86, 80};
    inline const sf::Color FLOOR_COLOR = {46, 42, 38};
    inline const sf::Color CROSSHAIR_COLOR = {255, 255, 255};
    inline const sf::Color AMMO_HUD_COLOR = {235, 230, 220};
    inline const sf::Color AMMO_HUD_LOW_COLOR = {220, 90, 70};
    inline const sf::Color AMMO_HUD_RELOADING_COLOR = {235, 190, 90};
    inline const sf::Color AMMO_HUD_OUTLINE_COLOR = {15, 13, 12, 220};
    inline const sf::Color RELOAD_INDICATOR_COLOR = {235, 190, 90};
    inline const sf::Color OVERLAY_BACKGROUND_COLOR = {0, 0, 0, 150};
    inline const sf::Color DEBUG_DETECTION_COLOR = {240, 200, 60};
    inline const sf::Color DEBUG_CHASING_COLOR = {240, 80, 60};
    inline const sf::Color DEBUG_ATTACK_RANGE_COLOR = {255, 140, 40};
    inline const sf::Color DEBUG_BLAST_COLOR = {230, 80, 230};
}
