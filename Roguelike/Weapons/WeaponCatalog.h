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

    constexpr float SHOT_NOISE_RADIUS = 560.f;
    constexpr float QUIET_NOISE_RADIUS = 200.f;
    constexpr float BLAST_NOISE_RADIUS = 900.f;

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
        float noiseRadius = SHOT_NOISE_RADIUS;
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
            "smg_suppressed", u8"ПП с глушителем", 57.60f, 12.16f, 0.60f, 0.35f, BulletKind::Pistol, AmmoKind::Smg, 30, 1.30f, "smg_silenced_shot",
            "smg_silenced_reload", nullptr, nullptr, nullptr, QUIET_NOISE_RADIUS
        },
        {"glock", u8"Глок", 32.00f, 11.68f, 0.55f, 0.65f, BulletKind::Pistol, AmmoKind::Pistol, 17, 1.10f, "glock_shot", "glock_reload"},
        {"deagle", u8"Дигл", 38.88f, 12.00f, 1.40f, 1.20f, BulletKind::Pistol, AmmoKind::Pistol, 7, 1.40f, "deagle_shot", "deagle_reload"},
        {
            "pistol_suppressed", u8"Пистолет с глушителем", 56.00f, 12.16f, 0.50f, 0.30f, BulletKind::Pistol, AmmoKind::Pistol, 12, 1.20f, "pistol_silenced_shot",
            "pistol_silenced_reload", nullptr, nullptr, nullptr, QUIET_NOISE_RADIUS
        },
        {"knife", u8"Нож", 42.40f, 15.36f, 0.00f, 0.00f, BulletKind::Pistol, AmmoKind::None, 0, 0.00f, nullptr, nullptr, &KNIFE_MELEE},
        {"bat", u8"Бита", 55.20f, 12.32f, 0.00f, 0.00f, BulletKind::Pistol, AmmoKind::None, 0, 0.00f, nullptr, nullptr, &BAT_MELEE},
        {"rpg", u8"РПГ", 76.00f, 12.16f, 1.80f, 1.60f, BulletKind::Rocket, AmmoKind::Rocket, 1, 2.60f, "rpg_shot", "rpg_reload", nullptr, nullptr, &RPG_EXPLOSIVE, BLAST_NOISE_RADIUS}
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

    /**
    *	Боевые характеристики ствола: урон за пулю, пауза между выстрелами,
    *	скорость пули и конус разброса в градусах.
    *	Ствол без строки в таблице стреляет силой стрелка.
    */
    struct FireProfile
    {
        WeaponId weapon;
        float damage;
        float cooldown;
        float speed;
        float spreadDegrees;
    };

    constexpr FireProfile WEAPON_FIRE[] = {
        {WeaponId::Ak47, 15.f, 0.100f, 1100.f, 6.0f},
        {WeaponId::M16, 11.f, 0.075f, 1150.f, 2.5f},
        {WeaponId::SmgSuppressed, 8.f, 0.067f, 950.f, 4.0f},
        {WeaponId::Glock, 18.f, 0.167f, 850.f, 3.0f},
        {WeaponId::Deagle, 45.f, 0.350f, 1000.f, 1.5f},
        {WeaponId::PistolSuppressed, 14.f, 0.200f, 850.f, 2.0f}
    };

    constexpr const FireProfile* FindFire(WeaponId id)
    {
        for (const FireProfile& fire : WEAPON_FIRE)
        {
            if (fire.weapon == id)
            {
                return &fire;
            }
        }

        return nullptr;
    }

    constexpr float SpreadOf(WeaponId id)
    {
        const FireProfile* fire = FindFire(id);

        return fire == nullptr ? 0.f : fire->spreadDegrees;
    }

    /**
    *	Вес ствола и вес того, что с ним таскают: магазины, патронташ, ракеты.
    *	Классы расходятся именно за счёт боекомплекта - сами стволы по массе
    *	отличаются куда слабее, чем снаряжение вокруг них.
    */
    struct WeaponWeight
    {
        WeaponId weapon;
        float kilograms;
        float ammoKilograms;
    };

    constexpr WeaponWeight WEAPON_WEIGHTS[] = {
        {WeaponId::Ak47, 4.30f, 2.40f},
        {WeaponId::M16, 3.90f, 2.20f},
        {WeaponId::ShotgunDouble, 3.20f, 0.80f},
        {WeaponId::ShotgunPump, 3.60f, 1.00f},
        {WeaponId::SmgSuppressed, 3.00f, 1.50f},
        {WeaponId::Glock, 0.90f, 0.90f},
        {WeaponId::Deagle, 2.00f, 1.05f},
        {WeaponId::PistolSuppressed, 1.20f, 0.90f},
        {WeaponId::Knife, 0.30f, 0.00f},
        {WeaponId::Bat, 1.00f, 0.00f},
        {WeaponId::Rpg, 7.00f, 4.40f}
    };

    static_assert(static_cast<int>(std::size(WEAPON_WEIGHTS)) == WEAPON_COUNT, "every weapon needs a weight");

    constexpr float LIGHTEST_WEAPON_WEIGHT = 0.30f;

    // Сколько шага съедает лишний килограмм ноши и где замедление упирается в пол.
    constexpr float PACE_COST_PER_KILOGRAM = 0.034f;
    constexpr float SLOWEST_PACE = 0.55f;

    constexpr const WeaponWeight* FindWeight(WeaponId id)
    {
        for (const WeaponWeight& entry : WEAPON_WEIGHTS)
        {
            if (entry.weapon == id)
            {
                return &entry;
            }
        }

        return nullptr;
    }

    /**
    *	Снаряжённый вес: ствол вместе с носимым боекомплектом.
    */
    constexpr float WeightOf(WeaponId id)
    {
        const WeaponWeight* entry = FindWeight(id);

        return entry == nullptr ? LIGHTEST_WEAPON_WEIGHT : entry->kilograms + entry->ammoKilograms;
    }

    /**
    *	Во сколько раз ноша укорачивает шаг: налегке единица, с РПГ около двух третей.
    */
    constexpr float MovePaceOf(WeaponId id)
    {
        float pace = 1.f - (WeightOf(id) - LIGHTEST_WEAPON_WEIGHT) * PACE_COST_PER_KILOGRAM;

        return pace < SLOWEST_PACE ? SLOWEST_PACE : pace;
    }

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
            return {1, SpreadOf(id), damage, speed, cooldown};
        }

        return {spread->pellets, spread->coneDegrees, damage * spread->damageScale, speed * spread->speedScale, cooldown * spread->cooldownScale};
    }

    /**
    *	Для того, кто раскрывает ствол полностью: урон и темп берутся из таблицы огня,
    *	а переданные значения остаются запасными для стволов без своей строки.
    */
    constexpr ShotProfile MakeWeaponShotProfile(WeaponId id, float damage, float speed, float cooldown)
    {
        const FireProfile* fire = FindFire(id);
        if (fire == nullptr)
        {
            return MakeShotProfile(id, damage, speed, cooldown);
        }

        return MakeShotProfile(id, fire->damage, fire->speed, fire->cooldown);
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
