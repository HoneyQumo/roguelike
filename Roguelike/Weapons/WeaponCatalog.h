#pragma once

#include <string_view>

#include <iterator>
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
        Pellet = 2,
        Rocket = 3
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
        MeleeAttackProfile quick;
        MeleeAttackProfile heavy;
        const char* hitSound;
        int hitSoundVariants;
    };

    struct SpreadDefinition
    {
        int pellets;
        float coneDegrees;
        float damageScale;
        float speedScale;
        float cooldownScale;
    };

    struct ExplosiveDefinition
    {
        float radius;
        float edgeDamagePart;
        float selfDamagePart;
        float damageScale;
        float speedScale;
        float cooldownScale;
        float lifetime;
        float colliderSize;
    };

    constexpr MeleeDefinition KNIFE_MELEE = {{1.00f, 1.00f, 52.f, 70.f, 0.12f}, {2.00f, 3.60f, 60.f, 110.f, 0.25f}, "knife_hit", 3};
    constexpr MeleeDefinition BAT_MELEE = {{1.40f, 1.40f, 68.f, 80.f, 0.18f}, {2.80f, 5.20f, 78.f, 140.f, 0.35f}, "bat_hit", 5};

    constexpr SpreadDefinition SHOTGUN_DOUBLE_SPREAD = {6, 32.f, 1.08f, 0.85f, 1.20f};
    constexpr SpreadDefinition SHOTGUN_PUMP_SPREAD = {5, 22.f, 1.12f, 0.85f, 2.20f};

    constexpr ExplosiveDefinition RPG_EXPLOSIVE = {96.f, 0.30f, 0.45f, 4.80f, 0.55f, 1.00f, 2.40f, 22.f};

    struct WeaponDefinition
    {
        const char* id;
        const char* name;
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

        // Свойства, которые есть не у каждого ствола.
        const MeleeDefinition* melee = nullptr;
        const SpreadDefinition* spread = nullptr;
        const ExplosiveDefinition* explosive = nullptr;
    };

    constexpr WeaponDefinition WEAPONS[] = {
        {"ak47", u8"АК-47", 58.56f, 12.16f, 1.00f, 1.00f, BulletKind::Rifle, AmmoKind::Rifle, 30, 1.60f, "ak47_shot", "ak47_reload"},
        {"m16", u8"М16", 60.80f, 12.16f, 0.85f, 0.90f, BulletKind::Rifle, AmmoKind::Rifle, 30, 1.50f, "m16_shot", "m16_reload"},
        {
            "shotgun_double", u8"Дробовик двуствольный", 53.60f, 12.16f, 1.50f, 1.35f, BulletKind::Pellet, AmmoKind::Shell, 2, 1.30f, "shotgun_double_shot",
            "shotgun_double_reload", nullptr, &SHOTGUN_DOUBLE_SPREAD
        },
        {
            "shotgun_pump", u8"Дробовик помповый", 55.20f, 11.52f, 1.35f, 1.25f, BulletKind::Pellet, AmmoKind::Shell, 6, 1.90f, "shotgun_pump_shot",
            "shotgun_pump_reload", nullptr, &SHOTGUN_PUMP_SPREAD
        },
        {
            "smg_suppressed", u8"ПП с глушителем", 57.60f, 12.16f, 0.60f, 0.35f, BulletKind::Pistol, AmmoKind::Smg, 25, 1.30f, "smg_silenced_shot",
            "smg_silenced_reload"
        },
        {"glock", u8"Глок", 32.00f, 11.68f, 0.55f, 0.65f, BulletKind::Pistol, AmmoKind::Pistol, 17, 1.10f, "glock_shot", "glock_reload"},
        {"deagle", u8"Дигл", 38.88f, 12.00f, 1.40f, 1.20f, BulletKind::Pistol, AmmoKind::Pistol, 7, 1.40f, "deagle_shot", "deagle_reload"},
        {
            "pistol_suppressed", u8"Пистолет с глушителем", 56.00f, 12.16f, 0.50f, 0.30f, BulletKind::Pistol, AmmoKind::Pistol, 12, 1.20f, "pistol_silenced_shot",
            "pistol_silenced_reload"
        },
        {"knife", u8"Нож", 42.40f, 15.36f, 0.00f, 0.00f, BulletKind::Pistol, AmmoKind::None, 0, 0.00f, nullptr, nullptr, &KNIFE_MELEE},
        {"bat", u8"Бита", 55.20f, 12.32f, 0.00f, 0.00f, BulletKind::Pistol, AmmoKind::None, 0, 0.00f, nullptr, nullptr, &BAT_MELEE},
        {"rpg", u8"РПГ", 76.00f, 12.16f, 1.80f, 1.60f, BulletKind::Rocket, AmmoKind::Rocket, 1, 2.60f, "rpg_shot", "rpg_reload", nullptr, nullptr, &RPG_EXPLOSIVE}
    };

    constexpr int WEAPON_COUNT = static_cast<int>(std::size(WEAPONS));
    constexpr int WEAPON_ATLAS_FRAMES = WEAPON_VARIANTS * WEAPON_COUNT;

    static_assert(WEAPON_COUNT == static_cast<int>(WeaponId::Rpg) + 1, "WEAPONS must have a row for every WeaponId");

    struct ShotProfile
    {
        int pellets;
        float coneDegrees;
        float damage;
        float speed;
        float cooldown;
    };

    struct AmmoKindName
    {
        const char* name;
        AmmoKind kind;
    };

    constexpr AmmoKindName AMMO_KIND_NAMES[] = {
        {"rifle", AmmoKind::Rifle},
        {"smg", AmmoKind::Smg},
        {"pistol", AmmoKind::Pistol},
        {"shell", AmmoKind::Shell},
        {"rocket", AmmoKind::Rocket}
    };

    constexpr bool TryGetAmmoKind(std::string_view name, AmmoKind& outKind)
    {
        for (const AmmoKindName& ammo : AMMO_KIND_NAMES)
        {
            if (name == ammo.name)
            {
                outKind = ammo.kind;
                return true;
            }
        }

        return false;
    }

    constexpr bool TryGetWeaponId(std::string_view id, WeaponId& outId)
    {
        for (int index = 0; index < WEAPON_COUNT; index++)
        {
            if (id == WEAPONS[index].id)
            {
                outId = static_cast<WeaponId>(index);
                return true;
            }
        }

        return false;
    }

    constexpr const WeaponDefinition& GetWeapon(WeaponId id)
    {
        return WEAPONS[static_cast<int>(id)];
    }

    constexpr const MeleeDefinition* FindMelee(WeaponId id)
    {
        return GetWeapon(id).melee;
    }

    constexpr bool IsMelee(WeaponId id)
    {
        return FindMelee(id) != nullptr;
    }

    constexpr const SpreadDefinition* FindSpread(WeaponId id)
    {
        return GetWeapon(id).spread;
    }

    constexpr const ExplosiveDefinition* FindExplosive(WeaponId id)
    {
        return GetWeapon(id).explosive;
    }

    constexpr bool IsExplosive(WeaponId id)
    {
        return FindExplosive(id) != nullptr;
    }

    constexpr ShotProfile MakeShotProfile(WeaponId id, float damage, float speed, float cooldown)
    {
        const ExplosiveDefinition* explosive = FindExplosive(id);
        if (explosive != nullptr)
        {
            return {1, 0.f, damage * explosive->damageScale, speed * explosive->speedScale, cooldown * explosive->cooldownScale};
        }

        const SpreadDefinition* spread = FindSpread(id);
        if (spread == nullptr)
        {
            return {1, 0.f, damage, speed, cooldown};
        }

        return {spread->pellets, spread->coneDegrees, damage * spread->damageScale, speed * spread->speedScale, cooldown * spread->cooldownScale};
    }

    constexpr int WeaponFrameIndex(WeaponId id, int variant)
    {
        return static_cast<int>(id) * WEAPON_VARIANTS + variant;
    }

    constexpr int AmmoKindKey(AmmoKind ammo)
    {
        return static_cast<int>(ammo);
    }
}
