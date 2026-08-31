#pragma once

#include "SpriteAtlas.h"

namespace RoguelikeGame
{
    /**	
    *	Раскладка оружия из Docs/Sprites/weapons.json.
    *	Строка — ствол, столбец — вариант хвата.
    */
    constexpr int WEAPON_FRAME_WIDTH = 160;
    constexpr int WEAPON_FRAME_HEIGHT = 64;
    constexpr int WEAPON_VARIANTS = 4;
    constexpr int WEAPON_COUNT = 11;
    constexpr int WEAPON_ATLAS_FRAMES = WEAPON_VARIANTS * WEAPON_COUNT;

    /**
     * 0 — обычный хват
     * 1 и 2 ради перезарядки
     * 3 — ствол без перчаток.
     */
    constexpr int WEAPON_DEFAULT_VARIANT = 0;
    constexpr int WEAPON_STOWED_VARIANT = 3;

    enum class BulletKind
    {
        Rifle = 0,
        Pistol = 1,
        Pellet = 2
    };

    /**	
    *	Тип боеприпаса из поля reload_item в Docs/Sprites/weapons.json.
    */
    enum class AmmoKind
    {
        None = 0,
        Rifle,
        Smg,
        Pistol,
        Shell,
        Rocket
    };

    enum class WeaponId
    {
        Ak47 = 0,
        M16,
        ShotgunDouble,
        ShotgunPump,
        SmgSuppressed,
        Glock,
        Deagle,
        PistolSuppressed,
        Knife,
        Bat,
        Rpg
    };

    struct WeaponDefinition
    {
        const char* id;
        const char* name;
        int row;
        // Смещение дульного среза от пивота тела, в пикселях кадра с осью Y вниз.
        float muzzleX;
        float muzzleY;
        float recoil;
        float flashScale;
        BulletKind bullet;
        AmmoKind ammo;
        int magazineSize;
        float reloadTime;
        const char* shotSound;
        const char* reloadSound;
    };

    constexpr WeaponDefinition WEAPONS[WEAPON_COUNT] = {
        {"ak47", u8"АК-47", 0, 58.56f, 12.16f, 1.00f, 1.00f, BulletKind::Rifle, AmmoKind::Rifle, 30, 1.60f, "ak47_shot", "ak47_reload"},
        {"m16", u8"М16", 1, 60.80f, 12.16f, 0.85f, 0.90f, BulletKind::Rifle, AmmoKind::Rifle, 30, 1.50f, "m16_shot", "m16_reload"},
        {
            "shotgun_double", u8"Дробовик двуствольный", 2, 53.60f, 12.16f, 1.50f, 1.35f, BulletKind::Pellet, AmmoKind::Shell, 2, 1.30f, "shotgun_double_shot",
            "shotgun_double_reload"
        },
        {
            "shotgun_pump", u8"Дробовик помповый", 3, 55.20f, 11.52f, 1.35f, 1.25f, BulletKind::Pellet, AmmoKind::Shell, 6, 1.90f, "shotgun_pump_shot",
            "shotgun_pump_reload"
        },
        {
            "smg_suppressed", u8"ПП с глушителем", 4, 57.60f, 12.16f, 0.60f, 0.35f, BulletKind::Pistol, AmmoKind::Smg, 25, 1.30f, "smg_silenced_shot",
            "smg_silenced_reload"
        },
        {"glock", u8"Глок", 5, 32.00f, 11.68f, 0.55f, 0.65f, BulletKind::Pistol, AmmoKind::Pistol, 17, 1.10f, "glock_shot", "glock_reload"},
        {"deagle", u8"Дигл", 6, 38.88f, 12.00f, 1.40f, 1.20f, BulletKind::Pistol, AmmoKind::Pistol, 7, 1.40f, "deagle_shot", "deagle_reload"},
        {
            "pistol_suppressed", u8"Пистолет с глушителем", 7, 56.00f, 12.16f, 0.50f, 0.30f, BulletKind::Pistol, AmmoKind::Pistol, 12, 1.20f, "pistol_silenced_shot",
            "pistol_silenced_reload"
        },
        {"knife", u8"Нож", 8, 42.40f, 15.36f, 0.00f, 0.00f, BulletKind::Pistol, AmmoKind::None, 0, 0.00f, nullptr, nullptr},
        {"bat", u8"Бита", 9, 55.20f, 12.32f, 0.00f, 0.00f, BulletKind::Pistol, AmmoKind::None, 0, 0.00f, nullptr, nullptr},
        {"rpg", u8"РПГ", 10, 76.00f, 12.16f, 1.80f, 1.60f, BulletKind::Rifle, AmmoKind::Rocket, 1, 2.60f, "rpg_shot", "rpg_reload"}
    };

    struct MeleeAttackProfile
    {
        float damageScale;
        float chargedDamageScale;
        float range;
        float arcDegrees;
        float recovery;
    };

    struct MeleeDefinition
    {
        WeaponId weapon;
        MeleeAttackProfile quick;
        MeleeAttackProfile heavy;
        const char* hitSound;
        int hitSoundVariants;
    };

    constexpr int MELEE_WEAPON_COUNT = 2;

    constexpr MeleeDefinition MELEE_WEAPONS[MELEE_WEAPON_COUNT] = {
        {
            WeaponId::Knife,
            {1.00f, 1.00f, 52.f, 70.f, 0.12f},
            {2.00f, 3.60f, 60.f, 110.f, 0.25f},
            "knife_hit", 3
        },
        {
            WeaponId::Bat,
            {1.40f, 1.40f, 68.f, 80.f, 0.18f},
            {2.80f, 5.20f, 78.f, 140.f, 0.35f},
            "bat_hit", 5
        }
    };

    constexpr const WeaponDefinition& GetWeapon(WeaponId id)
    {
        return WEAPONS[static_cast<int>(id)];
    }

    constexpr const MeleeDefinition* FindMelee(WeaponId id)
    {
        for (const MeleeDefinition& melee : MELEE_WEAPONS)
        {
            if (melee.weapon == id)
            {
                return &melee;
            }
        }

        return nullptr;
    }

    constexpr bool IsMelee(WeaponId id)
    {
        return FindMelee(id) != nullptr;
    }

    constexpr int WeaponFrameIndex(WeaponId id, int variant)
    {
        return GetWeapon(id).row * WEAPON_VARIANTS + variant;
    }

    constexpr int AmmoKindKey(AmmoKind ammo)
    {
        return static_cast<int>(ammo);
    }
}
