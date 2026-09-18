#pragma once

#include <string>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <CameraComponent.h>
#include "ItemDefinition.h"
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

    // Кусок полотна размером с комнату: весь массив вершин либо на экране, либо нет,
    // и на большой карте один массив гоняет десятки тысяч вершин ради двухсот видимых клеток.
    constexpr int TILE_CHUNK = 32;

    // Обзор игрока в клетках. Закрывает угол экрана целиком: темноту рисуют стены,
    // а не дуга радиуса посреди комнаты - плавного края у нас пока нет.
    // Заодно дальше любого врага: стреляющего по тебе видно.
    constexpr int FOG_SIGHT_RADIUS = 12;

    // Насколько гаснет разведанное: планировка читается, детали нет.
    constexpr float FOG_KNOWN_LIGHT = 0.42f;

    // Доля радиуса, внутри которой светит в полную силу, и яркость на самом краю.
    // Край светлее памяти: то, что видно сейчас, не должно быть темнее того, что запомнилось.
    constexpr float FOG_FULL_PART = 0.55f;
    constexpr float FOG_EDGE_LIGHT = 0.55f;
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

    // Мёртвая зона под stopDistance: ближе враг пятится. Взята абсолютом,
    // а не долей: у ножевика она шире его же stopDistance, и отход ему недоступен.
    constexpr float ENEMY_COMFORT_DEAD_ZONE = 60.f;
    constexpr int ENEMY_BACK_OFF_RADIUS = 5;
    constexpr int ENEMY_COVER_RADIUS = 6;

    // Запас вглубь тени: на самом краю укрытия хватает шага игрока, чтобы врага снова было видно.
    constexpr float ENEMY_COVER_MARGIN = 0.5f * TILE_SIZE;

    // Доля магазина, ниже которой враг уже ищет, где перезарядиться.
    constexpr float WEAPON_LOW_MAGAZINE_SHARE = 0.3f;

    // Клетка за спиной у игрока формально дальше, но идти туда - значит пройти сквозь него.
    constexpr float BACK_OFF_COSINE = 0.f;
    constexpr float BACK_OFF_GAIN_STEP = 0.5f * TILE_SIZE;

    // Задним ходом на полной скорости погони читается как баг.
    constexpr float ENEMY_BACK_OFF_PACE = 0.6f;

    // Потолок на дорогу к точке тревоги: с запасом на круговой обход, но не навечно.
    constexpr float SEARCH_TRAVEL_TIME = 12.f;
    constexpr float SEARCH_LOOK_SPREAD = 1.6f;
    constexpr float SEARCH_ESCAPE_COSINE = -0.2f;
    constexpr float SEARCH_ESCAPE_STEP = 48.f;

    constexpr int CHARACTER_SPRITE_SIZE = CHARACTER_FRAME_SIZE;

    constexpr float CHARACTER_COLLIDER_SIZE = 30.f;

    // Скорость налегке: настоящую даёт вес ствола в руках, см. MovePaceOf.
    constexpr float PLAYER_SPEED = 270.f;
    constexpr float PLAYER_RUN_SPEED_MULTIPLIER = 1.5f;
    constexpr float PLAYER_MAX_HEALTH = 100.f;
    constexpr float PLAYER_ARMOR = 40.f;
    constexpr float PLAYER_ARMOR_CAP = 120.f;

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
    constexpr auto INVENTORY_EQUIP_HINT = u8"[Enter] Экипировать";
    constexpr auto INVENTORY_SLOT_HINT = u8" · [1-%d] в слот";
    constexpr auto INVENTORY_BELT_HINT = u8" · [%d-%d] на пояс";
    constexpr auto INVENTORY_DROP_HINT = u8"[%c] Выбросить";
    constexpr auto INVENTORY_HINT_SEPARATOR = u8" · ";

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
    /**
    *	Ядро взрыва: внутри него урон полный и от укрытия толку нет.
    *	Радиус подобран так, чтобы стоящий вплотную к машине попадал в ядро.
    */
    constexpr float EXPLOSION_CORE_RADIUS = 1.2f * TILE_SIZE;

    constexpr float SHOT_FORWARD_OFFSET = 0.5f * CHARACTER_COLLIDER_SIZE + 4.f;

    inline XYZEngine::Vector2Df ShotOffset(const WeaponDefinition& weapon)
    {
        return {SHOT_FORWARD_OFFSET, ToWorldOffset(weapon.muzzleX, weapon.muzzleY).y};
    }

    constexpr int GROUND_RENDER_LAYER = 0;
    // Накладка ложится поверх пола, но под всем, что на нём лежит.
    constexpr int OVERLAY_RENDER_LAYER = 5;
    constexpr int BLOOD_RENDER_LAYER = 10;
    constexpr int PROP_DEBRIS_RENDER_LAYER = 15;
    constexpr int ITEM_RENDER_LAYER = 25;
    constexpr int CORPSE_RENDER_LAYER = 20;
    constexpr int ENEMY_RENDER_LAYER = 30;
    constexpr int STOWED_WEAPON_RENDER_LAYER = 40;
    constexpr int PLAYER_RENDER_LAYER = 50;
    constexpr int FIRE_RENDER_LAYER = 55;
    constexpr int EFFECT_RENDER_LAYER = 60;
    constexpr int UI_RENDER_LAYER = 70;

    constexpr int CROSSHAIR_SIZE = 32;
    constexpr int RELOAD_MAG_FRAME_SIZE = 64;
    constexpr int RELOAD_MAG_FRAMES = 30;
    constexpr int RELOAD_INDICATOR_SIZE = 44;

    constexpr int AMMO_HUD_FONT_SIZE = 30;
    constexpr int AMMO_HUD_NAME_FONT_SIZE = 18;

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

    constexpr float WEAPON_ROW_SLOT_SIZE = 64.f;
    constexpr float WEAPON_ROW_SLOT_GAP = 8.f;
    constexpr float WEAPON_ROW_MARGIN_Y = 24.f;

    constexpr int QUICK_BELT_HOOKS = 3;

    // Зазор между тройками: глаз читает «руки» и «пояс» как две группы, а не шесть ячеек.
    constexpr float BELT_ROW_GAP = 28.f;

    // Без задержки одно нажатие успевает выпить две аптечки за соседние кадры.
    constexpr float QUICK_BELT_COOLDOWN = 0.35f;
    constexpr int WEAPON_ROW_KEY_FONT_SIZE = 14;
    constexpr int WEAPON_ROW_COUNT_FONT_SIZE = 12;

    // Цифра клавиши и счётчик делят низ ячейки, а не лежат друг на друге.
    constexpr float WEAPON_ROW_KEY_WIDTH = 18.f;
    constexpr float WEAPON_ROW_COUNT_WIDTH = WEAPON_ROW_SLOT_SIZE - WEAPON_ROW_KEY_WIDTH - 2.f;
    constexpr float WEAPON_ROW_KEY_HEIGHT = 18.f;

    // Цифра берётся из привязки, а не пишется руками: подписи негде разойтись с управлением.
    constexpr int DigitOfKey(sf::Keyboard::Key key)
    {
        return key >= sf::Keyboard::Num0 && key <= sf::Keyboard::Num9
            ? static_cast<int>(key) - static_cast<int>(sf::Keyboard::Num0) : 0;
    }

    // Буква берётся из привязки по той же причине, что и цифра.
    constexpr char LetterOfKey(sf::Keyboard::Key key)
    {
        return key >= sf::Keyboard::A && key <= sf::Keyboard::Z
            ? static_cast<char>('A' + static_cast<int>(key) - static_cast<int>(sf::Keyboard::A)) : '?';
    }

    // Красная зона обоймы
    constexpr float AMMO_HUD_LOW_PART = 0.25f;

    constexpr int OVERLAY_TITLE_FONT_SIZE = 56;
    constexpr int OVERLAY_HINT_FONT_SIZE = 24;
    constexpr float OVERLAY_LINE_GAP = 36.f;
    constexpr float OVERLAY_LINE_WIDTH = 720.f;
    constexpr int HUD_NOTICE_FONT_SIZE = 22;
    constexpr float HUD_NOTICE_MARGIN_Y = 120.f;
    constexpr float HUD_NOTICE_TIME = 2.f;
    // Панель волн: заголовок, полоса по всей осаде и счётчик тех, кто ещё на хвосте.
    constexpr float WAVE_HUD_MARGIN_Y = 18.f;
    constexpr float WAVE_HUD_WIDTH = 300.f;
    constexpr int WAVE_HUD_TITLE_FONT_SIZE = 22;
    constexpr int WAVE_HUD_COUNT_FONT_SIZE = 18;
    constexpr float WAVE_HUD_BAR_HEIGHT = 10.f;
    constexpr float WAVE_HUD_GAP = 6.f;
    constexpr float WAVE_HUD_TITLE_HEIGHT = WAVE_HUD_TITLE_FONT_SIZE * AMMO_HUD_LINE_HEIGHT;
    constexpr float WAVE_HUD_COUNT_HEIGHT = WAVE_HUD_COUNT_FONT_SIZE * AMMO_HUD_LINE_HEIGHT;
    constexpr float WAVE_HUD_HEIGHT = WAVE_HUD_TITLE_HEIGHT + WAVE_HUD_BAR_HEIGHT + WAVE_HUD_COUNT_HEIGHT + 2.f * WAVE_HUD_GAP;
    inline const sf::Color WAVE_HUD_COLOR = {235, 225, 210};
    inline const sf::Color WAVE_HUD_BAR_COLOR = {215, 90, 60};
    inline const sf::Color WAVE_HUD_CALM_COLOR = {130, 190, 140};
    constexpr auto WAVE_HUD_TITLE = u8"ВОЛНА ";
    constexpr auto WAVE_HUD_OF = u8" / ";
    constexpr auto WAVE_HUD_LEFT = u8"Осталось: ";
    constexpr auto WAVE_HUD_CALM = u8"Затишье";
    constexpr auto WAVE_HUD_NEXT_IN = u8"Следующая волна через ";

    // Панель погони: полоса - путь до машины, подпись - насколько близко хвост.
    inline const sf::Color CHASE_HUD_CLOSE_COLOR = {215, 90, 60};
    inline const sf::Color CHASE_HUD_AWAY_COLOR = {130, 190, 140};
    constexpr auto CHASE_HUD_CLOSE = u8"Погоня близко";
    constexpr auto CHASE_HUD_AWAY = u8"Оторвался";

    constexpr auto INVENTORY_FULL_NOTICE = u8"Инвентарь полон";
    constexpr auto ITEM_REFUSED_NOTICE = u8"Сейчас это не пригодится";
    constexpr auto ITEM_USELESS_NOTICE = u8"Это ни на что не годится";
    constexpr auto ITEM_UNKNOWN_NOTICE = u8"Непонятная вещь";
    constexpr auto EQUIP_NOT_A_WEAPON_NOTICE = u8"Это не оружие";
    constexpr auto EQUIP_NO_SLOT_NOTICE = u8"Такого слота нет";
    constexpr auto EQUIP_WRONG_KIND_NOTICE = u8"Сюда это не встаёт";
    constexpr auto EQUIP_ALREADY_NOTICE = u8"Уже в руках";
    constexpr auto EQUIP_NO_WAY_BACK_NOTICE = u8"Некуда деть то, что снимаешь";
    constexpr auto BELT_REFUSED_NOTICE = u8"На пояс это не вешается";
    constexpr auto DROP_REFUSED_NOTICE = u8"Здесь не выложить";

    // Слой предмета ниже слоя игрока: брошенное точно под ноги пропадёт под спрайтом.
    constexpr float DROP_STEP = 34.f;

    constexpr float INVENTORY_NOTICE_TIME = 2.5f;

    constexpr const char* ItemRefuseText(ItemRefuseReason reason)
    {
        switch (reason)
        {
        case ItemRefuseReason::NoHandler:
            return ITEM_USELESS_NOTICE;
        case ItemRefuseReason::Unknown:
            return ITEM_UNKNOWN_NOTICE;
        default:
            return ITEM_REFUSED_NOTICE;
        }
    }
    constexpr auto BOSS_GATE_NOTICE = u8"Выход закрыт: сначала победи босса";
    constexpr auto BOSS_DEFEATED_NOTICE = u8"Босс повержен, выход открыт";
    constexpr auto INTERACT_PROMPT_PREFIX = u8"[E] Подобрать: ";
    constexpr auto CONTAINER_OPEN_PREFIX = u8"[E] Открыть: ";
    constexpr auto CONTAINER_LOCKED_PREFIX = u8"Заперто: нужен ";
    constexpr auto DOOR_OPEN_PROMPT = u8"[F] Открыть дверь";
    constexpr auto LEVER_PROMPT = u8"[E] Дёрнуть рычаг";
    constexpr auto HATCH_FLEE_PROMPT = u8"[F] Сбежать из тюрьмы";
    constexpr auto DOOR_OPEN_SOUND = "door_open";
    constexpr auto LEVER_SOUND = "lever";
    constexpr auto HATCH_SOUND = "hatch_open";
    constexpr float DOOR_VOLUME = 45.f;
    constexpr float LEVER_VOLUME = 55.f;
    constexpr float HATCH_VOLUME = 70.f;
    constexpr auto DOOR_LOCKED_PREFIX = u8"Дверь заперта: нужен ";
    constexpr auto DOOR_UNKNOWN_KEY_NAME = u8"ключ";
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
    constexpr float ARMOR_BAR_HEIGHT = 3.f;
    constexpr float ARMOR_BAR_GAP = 1.f;
    inline const sf::Color ARMOR_BAR_COLOR = {170, 180, 200};

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
    constexpr float VOICE_VOLUME = 70.f;
    // Враг волны не появляется ближе полутора клеток к игроку.
    // Вода перебирает кадры медленно: рябь, а не мельтешение.
    constexpr float WATER_FRAME_TIME = 0.28f;
    constexpr auto WAVE_DIRECTOR_OBJECT_NAME = "WaveDirector";
    constexpr auto PURSUIT_OBJECT_NAME = "Pursuit";

    // Погоня выходит из-за спины: ближе - видно рождение, дальше - не догонит.
    constexpr float PURSUIT_SPAWN_GAP = 7.f * TILE_SIZE;
    constexpr float PURSUIT_SPAWN_REACH = 15.f * TILE_SIZE;

    // Топчешься на месте - каждые столько секунд на хвосте становится на одного больше.
    constexpr float PURSUIT_GROW_TIME = 6.f;
    constexpr float PURSUIT_RELAX_STEP = 16.f * TILE_SIZE;
    constexpr float PURSUIT_CLOSE_RANGE = 12.f * TILE_SIZE;


    constexpr auto ESCAPE_CAR_OBJECT_NAME = "EscapeCar";
    constexpr auto CUTSCENE_OBJECT_NAME = "Cutscene";

    // Сцена с люком: камера уезжает показать, что именно открыл рычаг.
    constexpr auto HATCH_SCENE_BEAT = "hatch_opens";
    constexpr float HATCH_SCENE_TRAVEL = 0.55f;
    constexpr float HATCH_SCENE_HOLD = 1.1f;
    constexpr auto HATCH_OBJECT_PREFIX = "Hatch_";
    constexpr auto ESCAPE_CAR_TEXTURE = "prop_car_van";
    constexpr auto ESCAPE_CAR_PROMPT = u8"[F] Уехать";
    constexpr float ESCAPE_CAR_WIDTH = 124.f;
    constexpr float ESCAPE_CAR_HEIGHT = 66.f;

    // Побег: сел, поехал, экран погас.
    constexpr auto ESCAPE_BEAT_BOARD = "board";
    constexpr auto ESCAPE_BEAT_DRIVE = "drive";
    constexpr auto ESCAPE_BEAT_LEAVE = "leave";
    constexpr float ESCAPE_BOARD_TIME = 0.8f;
    constexpr float ESCAPE_DRIVE_TIME = 4.f;
    constexpr float ESCAPE_LEAVE_TIME = 1.4f;
    constexpr float ESCAPE_CAR_SPEED = 540.f;
    constexpr float ESCAPE_CAR_PICKUP = 320.f;
    constexpr auto ESCAPE_NOTICE = u8"Гони!";

    // Приезд: машины на карте нет, пока герой не добежит до конца.
    constexpr auto ESCAPE_CAR_OPEN_TEXTURE = "escape_car_open";
    constexpr auto ESCAPE_CAR_ATLAS_FILE = "Resources/Textures/props_bridge.png";
    constexpr int ESCAPE_CAR_OPEN_FRAME_LEFT = 1164;
    constexpr int ESCAPE_CAR_OPEN_FRAME_SIDE = 136;

    // Кадр с дверью вдвое выше кузова: центр машины остаётся на месте.
    constexpr float ESCAPE_CAR_OPEN_HEIGHT = ESCAPE_CAR_HEIGHT * 2.f;

    constexpr auto ARRIVAL_BEAT_DRIVE = "car_drive";
    constexpr auto ARRIVAL_BEAT_DOOR = "car_door";
    constexpr float ARRIVAL_CALL_RANGE = 12.f * TILE_SIZE;
    constexpr float ARRIVAL_ENTRY_OFFSET = 16.f * TILE_SIZE;
    constexpr float ARRIVAL_LOOK_TIME = 0.8f;
    constexpr float ARRIVAL_DRIVE_TIME = 2.f;
    constexpr float ARRIVAL_DOOR_TIME = 0.9f;
    constexpr float ARRIVAL_SKID_START = 0.55f;
    constexpr float ARRIVAL_SKID_SLIDE = 28.f;
    constexpr float ARRIVAL_FACING_IN = 180.f;

    // Машина встаёт поперёк дороги дверью на запад - туда, откуда бежит игрок.
    constexpr float ARRIVAL_FACING_PARKED = 270.f;

    // Доворот записан как 360, а не как 0: вращение продолжает занос, а не отматывает его.
    constexpr float ESCAPE_FACING_OUT = 360.f;

    // Задняя ось и колея считаются от кузова: изменится машина - следы переедут сами.
    constexpr float CAR_REAR_AXLE_OFFSET = ESCAPE_CAR_WIDTH * 0.30f;
    constexpr float CAR_WHEEL_SPACING = ESCAPE_CAR_HEIGHT * 0.36f;

    // Дым и следы во время заноса.
    constexpr auto SMOKE_TEXTURE = "tire_smoke";
    constexpr auto SMOKE_TEXTURE_FILE = "Resources/Textures/smoke.png";
    constexpr float ARRIVAL_SMOKE_STEP = 0.03f;
    constexpr float ARRIVAL_SMOKE_SCALE = 0.75f;

    constexpr auto TIRE_MARK_OBJECT_NAME = "TireMark";
    constexpr auto TIRE_MARK_PROP = "skid_mark";
    constexpr int TIRE_MARK_RENDER_LAYER = 8;
    constexpr float TIRE_MARK_WIDTH = 18.f;

    // Резина на асфальте: не чёрная, иначе читается как дыра в полотне.
    inline const sf::Color TIRE_MARK_COLOR = sf::Color(22, 20, 22, 230);

    // Чем глубже занос, тем чернее полоса, но не до прозрачности.
    constexpr float TIRE_MARK_MIN_ALPHA = 0.55f;
    constexpr float ARRIVAL_MARK_STEP = 16.f;

    // Вставшая машина больше не чертит: последние крупицы хода - это возврат заноса,
    // и след от них ложится поперёк дороги.
    constexpr float ARRIVAL_MARK_MIN_SPEED = 160.f;

    constexpr auto CAR_ENGINE_SOUND = "car_engine";
    constexpr auto CAR_ENGINE_SOUND_FILE = "Resources/Audio/car_engine.wav";
    constexpr auto CAR_SKID_SOUND = "car_skid";
    constexpr auto CAR_SKID_SOUND_FILE = "Resources/Audio/car_skid.wav";
    constexpr float CAR_ENGINE_VOLUME = 28.f;
    constexpr float CAR_SKID_VOLUME = 42.f;
    /**
    *	Волна выходит из окна вокруг игрока: не под ногами, но и не за горизонтом.
    *	На длинной карте точки разбросаны на сотни клеток, и без окна волна
    *	рождается там, куда игрок придёт через минуту.
    */
    constexpr float WAVE_SPAWN_GAP = 7.f * TILE_SIZE;
    constexpr float WAVE_SPAWN_REACH = 15.f * TILE_SIZE;

    // Отставшие позади не держат следующую волну: погоня должна нагонять, а не ждать.
    constexpr float WAVE_KEEP_RANGE = 24.f * TILE_SIZE;

    // Сколько игрок должен пройти, чтобы его нагнала следующая волна.
    constexpr float WAVE_ADVANCE_STEP = 56.f * TILE_SIZE;

    constexpr auto WAVE_GATE_NOTICE = u8"Выход закрыт: отбей все волны";
    constexpr auto WAVE_STARTED_NOTICE = u8"Волна ";
    constexpr auto WAVE_OF_NOTICE = u8" из ";
    constexpr auto WAVE_CLEARED_NOTICE = u8" отбита";
    constexpr auto WAVES_DONE_NOTICE = u8"Волны отбиты, выход открыт";

    constexpr auto FIRE_OBJECT_NAME = "Fire";
    constexpr auto FIRE_BIG_TEXTURE = "fire_big";
    constexpr auto FIRE_SMALL_TEXTURE = "fire_small";
    constexpr auto FIRE_TEXTURE_FILE = "Resources/Textures/fire.png";

    // Огонь жжёт всех, кто в нём стоит, и бьёт не каждый кадр, а раз в такт.
    constexpr float FIRE_BEAT_TIME = 0.45f;
    constexpr float FIRE_RADIUS = 46.f;
    constexpr float FIRE_DAMAGE = 7.f;
    constexpr float EMBER_RADIUS = 26.f;
    constexpr float EMBER_DAMAGE = 4.f;

    constexpr float EMBER_FLIGHT_TIME = 0.35f;
    constexpr float FIRE_THROW_HOP = 26.f;
    constexpr float EMBER_NEAR_PART = 0.35f;
    constexpr float EMBER_BURN_MIN = 3.5f;
    constexpr float EMBER_BURN_MAX = 8.5f;
    constexpr int EMBER_MIN_COUNT = 3;
    constexpr int EMBER_MAX_COUNT = 7;
    constexpr float EMBER_SCATTER_PART = 0.85f;

    // На границе радиуса остаётся чувствительный удар, но не смертельный.
    constexpr float PROP_BLAST_EDGE_PART = 0.15f;

    // Машина рвёт заметно громче бочки - встряска это показывает.
    constexpr float HEAVY_BLAST_RADIUS = 4.f * TILE_SIZE;
    constexpr float PROP_BLAST_NOISE_SCALE = 3.5f;
    constexpr float PROP_NOISE_RADIUS = 320.f;
    constexpr float MELEE_HIT_VOLUME = 55.f;

    constexpr float HEAVY_CHARGED_GLOW = 0.22f;
    constexpr float HEAVY_CHARGED_GLOW_PERIOD = 0.18f;

    constexpr auto PLAYER_OBJECT_NAME = "Player";
    constexpr auto CAMERA_OBJECT_NAME = "Camera";
    constexpr auto DOOR_OBJECT_PREFIX = "Door_";
    constexpr auto ROOMS_OBJECT_NAME = "Rooms";
    constexpr int ROOM_WAKE_AHEAD = 1;
    constexpr int ROOM_NEIGHBOUR_GAP = 1;
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
    constexpr auto BOSS_FX_ATLAS_FILE = "Resources/Textures/boss_puppeteer_fx.png";
    constexpr auto PUPPETEER_RIFT_TEXTURE = "fx_puppeteer_rift";
    constexpr auto PUPPETEER_CLOUD_TEXTURE = "fx_puppeteer_cloud";
    constexpr auto PUPPETEER_SNAP_TEXTURE = "fx_puppeteer_snap";
    constexpr auto PUPPETEER_MARK_TEXTURE = "fx_puppeteer_mark";
    constexpr auto RELOAD_MAG_TEXTURE = "reload_mag";
    constexpr auto DOOR_LOCKED_TEXTURE = "door_locked";
    constexpr auto DOOR_OPEN_TEXTURE = "door_open";
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
    constexpr auto DOORS_ATLAS_FILE = "Resources/Textures/doors.png";
    constexpr auto HIT_FLASH_SHADER_FILE = "Resources/Shaders/hit_flash.frag";
    constexpr auto SHOT_SOUND_FILE = "Resources/Audio/shot.wav";
    constexpr auto DOOR_OPEN_SOUND_FILE = "Resources/Audio/door_open.wav";
    constexpr auto LEVER_SOUND_FILE = "Resources/Audio/lever.wav";
    constexpr auto HATCH_SOUND_FILE = "Resources/Audio/hatch_open.wav";
    constexpr auto FIXTURES_TEXTURE_FILE = "Resources/Textures/fixtures.png";
    constexpr auto HATCH_TEXTURE_PREFIX = "hatch_";
    constexpr auto LEVER_TEXTURE_PREFIX = "lever_";
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
    inline const sf::Color INVENTORY_NOTICE_COLOR = {230, 170, 90};
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
    inline const sf::Color AWARENESS_GAUGE_COLOR = {245, 215, 90};
    inline const sf::Color AWARENESS_GAUGE_ALARM_COLOR = {235, 65, 50};
    inline const sf::Color AWARENESS_GAUGE_BACK_COLOR = {18, 18, 22, 170};
    constexpr float AWARENESS_GAUGE_RIM = 2.f;
    inline const sf::Color DEBUG_DETECTION_COLOR = {240, 200, 60};
    inline const sf::Color DEBUG_CHASING_COLOR = {240, 80, 60};
    inline const sf::Color DEBUG_ATTACK_RANGE_COLOR = {255, 140, 40};
    inline const sf::Color DEBUG_BLAST_COLOR = {230, 80, 230};
    inline const sf::Color DEBUG_ROUTE_COLOR = {80, 220, 220};
    inline const sf::Color DEBUG_PATROL_COLOR = {90, 160, 240};
    inline const sf::Color DOOR_LOCKED_COLOR = {150, 120, 45};
    inline const sf::Color DOOR_OPEN_COLOR = {70, 60, 35};
    constexpr float DOOR_SWING_ANGLE = 90.f;
    constexpr float DOOR_SWING_TIME = 0.5f;
    constexpr float DOOR_REACH_MARGIN = 28.f;
    constexpr auto DOOR_LEAF_OBJECT_NAME = "Leaf";
    constexpr int DEBUG_VISION_CONE_STEPS = 12;
}
