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
    constexpr int WEAPON_VARIANTS = 3;
    constexpr int WEAPON_COUNT = 11;
    constexpr int WEAPON_ATLAS_FRAMES = WEAPON_VARIANTS * WEAPON_COUNT;

    // Столбец 0 — обычный хват. Столбцы 1 и 2 ради перезарядки.
    constexpr int WEAPON_DEFAULT_VARIANT = 0;

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
    };

    constexpr WeaponDefinition WEAPONS[WEAPON_COUNT] = {
        {"ak47", 0, 58.56f, 12.16f, 1.00f, 1.00f, BulletKind::Rifle, AmmoKind::Rifle, 30, 1.60f},
        {"m16", 1, 60.80f, 12.16f, 0.85f, 0.90f, BulletKind::Rifle, AmmoKind::Rifle, 30, 1.50f},
        {"shotgun_double", 2, 53.60f, 12.16f, 1.50f, 1.35f, BulletKind::Pellet, AmmoKind::Shell, 2, 1.30f},
        {"shotgun_pump", 3, 55.20f, 11.52f, 1.35f, 1.25f, BulletKind::Pellet, AmmoKind::Shell, 6, 1.90f},
        {"smg_suppressed", 4, 57.60f, 12.16f, 0.60f, 0.35f, BulletKind::Pistol, AmmoKind::Smg, 25, 1.30f},
        {"glock", 5, 32.00f, 11.68f, 0.55f, 0.65f, BulletKind::Pistol, AmmoKind::Pistol, 17, 1.10f},
        {"deagle", 6, 38.88f, 12.00f, 1.40f, 1.20f, BulletKind::Pistol, AmmoKind::Pistol, 7, 1.40f},
        {"pistol_suppressed", 7, 56.00f, 12.16f, 0.50f, 0.30f, BulletKind::Pistol, AmmoKind::Pistol, 12, 1.20f},
        {"knife", 8, 42.40f, 15.36f, 0.00f, 0.00f, BulletKind::Pistol, AmmoKind::None, 0, 0.00f},
        {"bat", 9, 55.20f, 12.32f, 0.00f, 0.00f, BulletKind::Pistol, AmmoKind::None, 0, 0.00f},
        {"rpg", 10, 76.00f, 12.16f, 1.80f, 1.60f, BulletKind::Rifle, AmmoKind::Rocket, 1, 2.60f}
    };

    constexpr const WeaponDefinition& GetWeapon(WeaponId id)
    {
        return WEAPONS[static_cast<int>(id)];
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
