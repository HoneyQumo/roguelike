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
    constexpr std::size_t PARTICLE_POOL_CAPACITY = 1024;
    constexpr sf::Keyboard::Key DEBUG_HEAL_KEY = sf::Keyboard::H;
    constexpr float DEBUG_HEAL_AMOUNT = 25.f;
    constexpr float HIT_STOP_LIGHT = 0.035f;
    constexpr float HIT_STOP_HEAVY = 0.07f;
    constexpr float DEATH_TIME_SCALE = 0.35f;
    constexpr float DEATH_SLOW_MOTION_TIME = 0.7f;
    constexpr float DEATH_SLOW_MOTION_BLEND = 0.6f;
    constexpr XYZEngine::CameraShake CAMERA_SHAKE_LIGHT = {5.f, 0.12f, 24.f};
    constexpr XYZEngine::CameraShake CAMERA_SHAKE_HEAVY = {13.f, 0.28f, 18.f};
    constexpr XYZEngine::CameraShake CAMERA_SHAKE_BLAST = {24.f, 0.45f, 14.f};

    constexpr float TILE_SIZE = 64.f;
    constexpr float ENEMY_ALERT_ARRIVE_DISTANCE = 48.f;
    constexpr float ENEMY_ALERT_POINT_DISTANCE = 420.f;
    constexpr float ENEMY_REPATH_INTERVAL = 0.4f;
    constexpr float ENEMY_LOOK_AROUND_TIME = 1.5f;
    constexpr float NAVIGATION_PROGRESS_WINDOW = 0.4f;
    constexpr float NAVIGATION_BLOCKED_TIME = 2.f;
    constexpr float NAVIGATION_MIN_PROGRESS = 0.4f;
    constexpr float ENEMY_ROUTE_ARRIVE_DISTANCE = 0.5f * TILE_SIZE;
    constexpr float PATROL_JOIN_DISTANCE = 4.f * TILE_SIZE;
    constexpr float LOOK_AIM_DISTANCE = 200.f;
    constexpr int SEARCH_SPOT_GAP = 3;
    constexpr float SEARCH_ARRIVE_DISTANCE = 20.f;
    constexpr float SEARCH_LOOK_SPREAD = 1.6f;
    constexpr float SEARCH_ESCAPE_COSINE = -0.2f;
    constexpr float SEARCH_ESCAPE_STEP = 48.f;

    constexpr int CHARACTER_SPRITE_SIZE = CHARACTER_FRAME_SIZE;

    constexpr float CHARACTER_COLLIDER_SIZE = 30.f;

    constexpr float PLAYER_SPEED = 250.f;
    constexpr float PLAYER_RUN_SPEED_MULTIPLIER = 1.5f;
    constexpr float PLAYER_MAX_HEALTH = 100.f;
    constexpr float PLAYER_ARMOR = 5.f;
    constexpr float PLAYER_ARMOR_CAP = 20.f;

    constexpr bool CanTakeArmor(float current, float cap)
    {
        return current < cap;
    }

    constexpr float ArmorAfterPlate(float current, float amount, float cap)
    {
        if (amount <= 0.f || current >= cap)
        {
            return current;
        }

        return current + amount > cap ? cap : current + amount;
    }

    constexpr float PLAYER_ATTACK_DAMAGE = 25.f;
    constexpr float PLAYER_ATTACK_COOLDOWN = 0.3f;
    constexpr float PLAYER_PROJECTILE_SPEED = 800.f;
    constexpr float PLAYER_MELEE_DAMAGE = 15.f;
    constexpr float PLAYER_HEAVY_LUNGE_SPEED = 150.f;

    constexpr float PLAYER_MAX_STAMINA = 100.f;
    constexpr float PLAYER_STAMINA_RUN_DRAIN = 24.f;
    constexpr float PLAYER_STAMINA_REGEN = 20.f;
    constexpr float PLAYER_STAMINA_REGEN_DELAY = 0.7f;
    constexpr float PLAYER_STAMINA_RUN_RESUME_PART = 0.25f;
    constexpr float PLAYER_ROLL_STAMINA = 25.f;
    constexpr float PLAYER_HEAVY_ATTACK_STAMINA = 20.f;

    constexpr float PLAYER_ROLL_SPEED = 900.f;
    constexpr float PLAYER_ROLL_MAX_STEP = 0.75f * CHARACTER_COLLIDER_SIZE;
    constexpr float PLAYER_ROLL_COOLDOWN = 0.35f;

    constexpr unsigned int PLAYER_COLLISION_LAYER = 1u << 1;
    constexpr unsigned int ENEMY_COLLISION_LAYER = 1u << 2;
    constexpr unsigned int ITEM_COLLISION_LAYER = 1u << 3;
    constexpr float ITEM_PICKUP_SIZE = 48.f;
    constexpr int INVENTORY_CAPACITY = 12;
    constexpr int INVENTORY_GRID_COLUMNS = 4;
    constexpr int INVENTORY_GRID_ROWS = 3;
    constexpr float INVENTORY_SLOT_SIZE = 96.f;
    constexpr float INVENTORY_SLOT_GAP = 12.f;
    constexpr float INVENTORY_WINDOW_PADDING = 28.f;
    constexpr float INVENTORY_TITLE_HEIGHT = 44.f;
    constexpr int INVENTORY_TITLE_FONT_SIZE = 28;
    constexpr int INVENTORY_SLOT_FONT_SIZE = 12;
    constexpr int INVENTORY_COUNT_FONT_SIZE = 18;
    constexpr float INVENTORY_ICON_PART = 0.52f;
    constexpr float INVENTORY_OPEN_TIME = 0.18f;
    constexpr float INVENTORY_SLIDE_OFFSET = 60.f;
    constexpr auto INVENTORY_TITLE = u8"ИНВЕНТАРЬ";
    constexpr float INVENTORY_HINT_HEIGHT = 30.f;
    constexpr int INVENTORY_HINT_FONT_SIZE = 18;
    constexpr auto INVENTORY_USE_HINT = u8"[Enter] Использовать";
    constexpr auto INVENTORY_EQUIP_HINT = u8"[Enter] Экипировать в слот ";

    constexpr int PLAYER_WEAPON_SLOTS = 3;
    constexpr int PLAYER_START_WEAPON_SLOT = 2;

    struct StartingSlot
    {
        bool hasWeapon;
        WeaponId id;
    };

    constexpr StartingSlot PLAYER_LOADOUT[PLAYER_WEAPON_SLOTS] = {
        {false, WeaponId::Ak47},
        {false, WeaponId::Glock},
        {true, WeaponId::Knife}
    };

    constexpr auto EMPTY_SLOT_NAME = u8"—";

    constexpr int PreferredWeaponSlot(WeaponId id)
    {
        if (IsMelee(id))
        {
            return PLAYER_WEAPON_SLOTS - 1;
        }

        AmmoKind ammo = GetWeapon(id).ammo;

        return ammo == AmmoKind::Rifle || ammo == AmmoKind::Rocket ? 0 : 1;
    }

    constexpr int NO_WEAPON_SLOT = -1;

    struct AmmoReserve
    {
        AmmoKind kind;
        int count;
    };

    constexpr AmmoReserve PLAYER_START_AMMO[] = {
        {AmmoKind::Rifle, 0},
        {AmmoKind::Smg, 0},
        {AmmoKind::Pistol, 0},
        {AmmoKind::Shell, 0},
        {AmmoKind::Rocket, 0}
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
    constexpr int BLOOD_RENDER_LAYER = 10;
    constexpr int PROP_DEBRIS_RENDER_LAYER = 15;
    constexpr int ITEM_RENDER_LAYER = 25;
    constexpr int CORPSE_RENDER_LAYER = 20;
    constexpr int ENEMY_RENDER_LAYER = 30;
    constexpr int STOWED_WEAPON_RENDER_LAYER = 40;
    constexpr int PLAYER_RENDER_LAYER = 50;
    constexpr int EFFECT_RENDER_LAYER = 60;
    constexpr int UI_RENDER_LAYER = 70;

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
    constexpr float AMMO_HUD_NAME_HEIGHT = AMMO_HUD_NAME_FONT_SIZE * AMMO_HUD_LINE_HEIGHT;
    constexpr float AMMO_HUD_AMMO_HEIGHT = AMMO_HUD_FONT_SIZE * AMMO_HUD_LINE_HEIGHT;
    constexpr float AMMO_HUD_WIDTH = 260.f;
    constexpr float VITALS_HUD_MARGIN_X = 26.f;
    constexpr float VITALS_HUD_MARGIN_Y = 22.f;
    constexpr float VITALS_HUD_WIDTH = 280.f;
    constexpr float VITALS_HUD_HEALTH_HEIGHT = 20.f;
    constexpr float VITALS_HUD_STAMINA_HEIGHT = 10.f;
    constexpr float VITALS_HUD_ARMOR_HEIGHT = 8.f;
    constexpr float VITALS_HUD_GAP = 6.f;
    constexpr float VITALS_HUD_CRITICAL_PART = 0.25f;
    constexpr float VITALS_HUD_LOW_PART = 0.5f;
    inline const sf::Color VITALS_HUD_BACK_COLOR = {20, 20, 20, 180};
    inline const sf::Color VITALS_HUD_HEALTH_COLOR = {90, 200, 90};
    inline const sf::Color VITALS_HUD_HEALTH_LOW_COLOR = {220, 190, 70};
    inline const sf::Color VITALS_HUD_HEALTH_CRITICAL_COLOR = {215, 60, 55};
    inline const sf::Color VITALS_HUD_STAMINA_COLOR = {80, 170, 230};
    inline const sf::Color VITALS_HUD_STAMINA_EMPTY_COLOR = {120, 120, 130};
    inline const sf::Color VITALS_HUD_ARMOR_COLOR = {170, 180, 200};
    constexpr float AMMO_HUD_HEIGHT = AMMO_HUD_NAME_HEIGHT + AMMO_HUD_AMMO_HEIGHT;

    // Красная зона обоймы
    constexpr float AMMO_HUD_LOW_PART = 0.25f;

    constexpr int OVERLAY_TITLE_FONT_SIZE = 56;
    constexpr int OVERLAY_HINT_FONT_SIZE = 24;
    constexpr float OVERLAY_LINE_GAP = 36.f;
    constexpr float OVERLAY_LINE_WIDTH = 720.f;
    constexpr int HUD_NOTICE_FONT_SIZE = 22;
    constexpr float HUD_NOTICE_MARGIN_Y = 120.f;
    constexpr float HUD_NOTICE_TIME = 2.f;
    constexpr auto INVENTORY_FULL_NOTICE = u8"Инвентарь полон";
    constexpr auto ITEM_REFUSED_NOTICE = u8"Сейчас это не пригодится";
    constexpr auto BOSS_GATE_NOTICE = u8"Выход закрыт: сначала победи босса";
    constexpr auto BOSS_DEFEATED_NOTICE = u8"Босс повержен, выход открыт";
    constexpr auto INTERACT_PROMPT_PREFIX = u8"[E] Подобрать: ";
    constexpr auto CONTAINER_OPEN_PREFIX = u8"[E] Открыть: ";
    constexpr auto CONTAINER_LOCKED_PREFIX = u8"Заперто: нужен ";
    constexpr int HUD_PROMPT_FONT_SIZE = 24;
    constexpr float HUD_PROMPT_MARGIN_Y = 170.f;
    constexpr sf::Keyboard::Key RESTART_KEY = sf::Keyboard::R;
    constexpr float LEVEL_FADE_OUT_TIME = 0.35f;
    constexpr float LEVEL_FADE_IN_TIME = 0.45f;
    inline const sf::Color LEVEL_FADE_COLOR = {8, 8, 10, 255};
    constexpr float GAME_OVER_DELAY = 1.5f;
    constexpr auto PAUSE_TITLE = u8"ПАУЗА";
    constexpr auto PAUSE_HINT = u8"Esc — продолжить";
    constexpr auto VICTORY_TITLE = u8"ПОБЕДА";
    constexpr auto VICTORY_HINT = u8"R — заново";
    constexpr auto GAME_OVER_TITLE = u8"ВЫ ПОГИБЛИ";
    constexpr auto GAME_OVER_HINT = u8"R — заново";

    constexpr float HEALTH_BAR_WIDTH = 48.f;
    constexpr float HEALTH_BAR_HEIGHT = 6.f;
    constexpr float HEALTH_BAR_OFFSET_Y = 30.f;

    constexpr float BOSS_HEALTH_BAR_WIDTH = 96.f;
    constexpr float BOSS_HEALTH_BAR_HEIGHT = 10.f;
    constexpr float BOSS_HEALTH_BAR_OFFSET_Y = 44.f;
    constexpr float BOSS_BASIC_ATTACK_TIME = 1.2f;
    constexpr float BOSS_RECOVERY_TIME = 0.9f;
    constexpr float BOSS_ENRAGE_ROAR_TIME = 0.96f;
    constexpr float BOSS_ENRAGE_SPEED_SCALE = 1.35f;
    constexpr float BOSS_ENRAGE_DAMAGE_SCALE = 1.25f;
    constexpr float BOSS_ENRAGE_PACE_SCALE = 0.7f;
    constexpr float BOSS_MUZZLE_DISTANCE = 40.f;
    constexpr float BOSS_MARK_SPEED = 260.f;
    constexpr float BOSS_MARK_TOUCH_RADIUS = 34.f;
    constexpr float BOSS_MARK_FLIGHT_TIME = 4.f;
    constexpr float BOSS_MARK_FUSE_TIME = 0.9f;
    constexpr int BOSS_MARK_FLIGHT_SIZE = 40;
    constexpr float RAGE_AURA_RADIUS = 34.f;
    constexpr float BOSS_RIFT_RADIUS = 48.f;
    constexpr float BOSS_MINION_RIFT_RADIUS = 26.f;
    inline const sf::Color BOSS_HEALTH_BAR_COLOR = {215, 60, 55};
    inline const sf::Color BOSS_ENRAGED_BAR_COLOR = {235, 140, 40};
    inline const sf::Color BOSS_CAST_MARK_COLOR = {235, 90, 60, 90};

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
    constexpr auto PARTICLES_OBJECT_NAME = "Particles";
    constexpr auto UI_ROOT_OBJECT_NAME = "Ui";
    constexpr auto LEVEL_EXIT_OBJECT_NAME = "LevelExit";
    constexpr auto CAST_MARK_OBJECT_NAME = "CastMark";
    constexpr auto BLOOD_POOL_OBJECT_NAME = "BloodPool";
    constexpr auto FX_OBJECT_NAME = "Fx";
    constexpr auto PROJECTILE_OBJECT_NAME = "Projectile";
    constexpr auto ROCKET_OBJECT_NAME = "Rocket";

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
    constexpr auto FX_ATLAS_TEXTURE = "fx_atlas";
    constexpr auto TILES_TEXTURE = "tiles";
    constexpr auto BOSS_FX_ATLAS_FILE = "Resources/Textures/boss_puppeteer_fx.png";
    constexpr auto PUPPETEER_RIFT_TEXTURE = "fx_puppeteer_rift";
    constexpr auto PUPPETEER_CLOUD_TEXTURE = "fx_puppeteer_cloud";
    constexpr auto PUPPETEER_SNAP_TEXTURE = "fx_puppeteer_snap";
    constexpr auto PUPPETEER_MARK_TEXTURE = "fx_puppeteer_mark";
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
    constexpr auto TILES_ATLAS_FILE = "Resources/Textures/tiles.png";
    constexpr auto HIT_FLASH_SHADER_FILE = "Resources/Shaders/hit_flash.frag";
    constexpr auto SHOT_SOUND_FILE = "Resources/Audio/shot.wav";
    constexpr auto HURT_SOUND_FILE = "Resources/Audio/hurt.wav";
    constexpr auto MAIN_THEME_FILE = "Resources/Audio/main_music_1.ogg";
    constexpr auto TEST_LEVEL_FILE = "Resources/Levels/test_level.config";
    constexpr auto LEVELS_CATALOG_FILE = "Resources/Levels/levels.config";
    constexpr float LOOT_DROP_SPREAD = 26.f;
    constexpr float ITEM_WORLD_SIZE = 40.f;
    constexpr unsigned char ITEM_ICON_ALPHA_THRESHOLD = 8;
    constexpr auto LOOT_CATALOG_FILE = "Resources/Loot/loot.config";
    constexpr auto PROPS_CATALOG_FILE = "Resources/Props/props.config";
    constexpr auto PROP_OBJECT_PREFIX = "Prop_";
    constexpr float CONTAINER_REACH_MARGIN = 24.f;
    constexpr auto ITEMS_CATALOG_FILE = "Resources/Items/items.config";
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
    inline const sf::Color LEVEL_EXIT_COLOR = {90, 190, 120, 200};
    inline const sf::Color LEVEL_EXIT_LOCKED_COLOR = {90, 90, 100, 160};
    inline const sf::Color INVENTORY_DIM_COLOR = {0, 0, 0, 150};
    inline const sf::Color INVENTORY_WINDOW_COLOR = {28, 28, 32, 235};
    inline const sf::Color INVENTORY_SLOT_EMPTY_COLOR = {44, 44, 50, 220};
    inline const sf::Color INVENTORY_SLOT_FILLED_COLOR = {70, 70, 80, 235};
    inline const sf::Color INVENTORY_SLOT_SELECTED_COLOR = {120, 150, 190, 245};
    inline const sf::Color INVENTORY_SLOT_OUTLINE_COLOR = {16, 16, 18, 255};
    inline const sf::Color DEBUG_DETECTION_COLOR = {240, 200, 60};
    inline const sf::Color DEBUG_CHASING_COLOR = {240, 80, 60};
    inline const sf::Color DEBUG_ATTACK_RANGE_COLOR = {255, 140, 40};
    inline const sf::Color DEBUG_BLAST_COLOR = {230, 80, 230};
    inline const sf::Color DEBUG_ROUTE_COLOR = {80, 220, 220};
    inline const sf::Color DEBUG_PATROL_COLOR = {90, 160, 240};
    constexpr int DEBUG_VISION_CONE_STEPS = 12;
}
