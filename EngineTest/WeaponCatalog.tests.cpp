#include "pch.h"
#include "WeaponCatalog.h"

using namespace RoguelikeGame;

namespace
{
	constexpr WeaponId ALL_WEAPONS[] = {
		WeaponId::Ak47, WeaponId::M16, WeaponId::ShotgunDouble, WeaponId::ShotgunPump,
		WeaponId::SmgSuppressed, WeaponId::Glock, WeaponId::Deagle, WeaponId::PistolSuppressed,
		WeaponId::Knife, WeaponId::Bat, WeaponId::Rpg
	};
}

static_assert(std::size(ALL_WEAPONS) == WEAPON_COUNT, "ALL_WEAPONS must list every WeaponId");
static_assert(std::size(WEAPONS) == WEAPON_COUNT, "WEAPONS size must match WEAPON_COUNT");

TEST(WeaponCatalogTests, RowMatchesWeaponIdForEveryWeapon)
{
	for (WeaponId id : ALL_WEAPONS)
	{
		EXPECT_EQ(GetWeapon(id).row, static_cast<int>(id)) << "weapon " << GetWeapon(id).id;
	}
}

TEST(WeaponCatalogTests, EveryWeaponHasIdAndName)
{
	for (WeaponId id : ALL_WEAPONS)
	{
		const WeaponDefinition& weapon = GetWeapon(id);
		ASSERT_NE(weapon.id, nullptr);
		ASSERT_NE(weapon.name, nullptr);
		EXPECT_STRNE(weapon.id, "");
		EXPECT_STRNE(weapon.name, "");
	}
}

TEST(WeaponCatalogTests, RangedWeaponsHaveMagazineAndSounds)
{
	for (WeaponId id : ALL_WEAPONS)
	{
		if (IsMelee(id))
		{
			continue;
		}

		const WeaponDefinition& weapon = GetWeapon(id);
		EXPECT_GT(weapon.magazineSize, 0) << weapon.id;
		EXPECT_GT(weapon.reloadTime, 0.f) << weapon.id;
		EXPECT_NE(weapon.ammo, AmmoKind::None) << weapon.id;
		ASSERT_NE(weapon.shotSound, nullptr) << weapon.id;
		ASSERT_NE(weapon.reloadSound, nullptr) << weapon.id;
	}
}

TEST(WeaponCatalogTests, MeleeWeaponsHaveNoAmmoButHaveHitSound)
{
	for (WeaponId id : ALL_WEAPONS)
	{
		if (!IsMelee(id))
		{
			continue;
		}

		const WeaponDefinition& weapon = GetWeapon(id);
		EXPECT_EQ(weapon.magazineSize, 0) << weapon.id;
		EXPECT_EQ(weapon.ammo, AmmoKind::None) << weapon.id;

		const MeleeDefinition* melee = FindMelee(id);
		ASSERT_NE(melee, nullptr) << weapon.id;
		ASSERT_NE(melee->hitSound, nullptr) << weapon.id;
		EXPECT_GT(melee->hitSoundVariants, 0) << weapon.id;
	}
}

TEST(WeaponCatalogTests, SideTablesPointAtExistingWeapons)
{
	for (const MeleeDefinition& melee : MELEE_WEAPONS)
	{
		EXPECT_EQ(FindMelee(melee.weapon), &melee);
	}
	for (const SpreadDefinition& spread : SPREAD_WEAPONS)
	{
		EXPECT_EQ(FindSpread(spread.weapon), &spread);
	}
	for (const ExplosiveDefinition& explosive : EXPLOSIVE_WEAPONS)
	{
		EXPECT_EQ(FindExplosive(explosive.weapon), &explosive);
	}
}

TEST(WeaponCatalogTests, MeleeAndExplosiveAreDisjoint)
{
	for (WeaponId id : ALL_WEAPONS)
	{
		EXPECT_FALSE(IsMelee(id) && IsExplosive(id)) << GetWeapon(id).id;
	}
}

TEST(WeaponCatalogTests, ShotProfileForPlainWeaponIsUnscaled)
{
	ShotProfile profile = MakeShotProfile(WeaponId::Ak47, 100.f, 800.f, 0.5f);

	EXPECT_EQ(profile.pellets, 1);
	EXPECT_FLOAT_EQ(profile.coneDegrees, 0.f);
	EXPECT_FLOAT_EQ(profile.damage, 100.f);
	EXPECT_FLOAT_EQ(profile.speed, 800.f);
	EXPECT_FLOAT_EQ(profile.cooldown, 0.5f);
}

TEST(WeaponCatalogTests, ShotProfileForShotgunUsesSpreadTable)
{
	const SpreadDefinition* spread = FindSpread(WeaponId::ShotgunPump);
	ASSERT_NE(spread, nullptr);

	ShotProfile profile = MakeShotProfile(WeaponId::ShotgunPump, 100.f, 800.f, 0.5f);

	EXPECT_EQ(profile.pellets, spread->pellets);
	EXPECT_FLOAT_EQ(profile.coneDegrees, spread->coneDegrees);
	EXPECT_FLOAT_EQ(profile.damage, 100.f * spread->damageScale);
}

TEST(WeaponCatalogTests, ShotProfileForExplosiveWinsOverSpread)
{
	const ExplosiveDefinition* explosive = FindExplosive(WeaponId::Rpg);
	ASSERT_NE(explosive, nullptr);

	ShotProfile profile = MakeShotProfile(WeaponId::Rpg, 100.f, 800.f, 0.5f);

	EXPECT_EQ(profile.pellets, 1);
	EXPECT_FLOAT_EQ(profile.damage, 100.f * explosive->damageScale);
}

TEST(WeaponCatalogTests, WeaponFrameIndexStaysInsideAtlas)
{
	for (WeaponId id : ALL_WEAPONS)
	{
		for (int variant = 0; variant < WEAPON_VARIANTS; variant++)
		{
			int index = WeaponFrameIndex(id, variant);
			EXPECT_GE(index, 0);
			EXPECT_LT(index, WEAPON_ATLAS_FRAMES) << GetWeapon(id).id << " variant " << variant;
		}
	}
}
