#include "pch.h"
#include "WeaponCatalog.h"
#include "GameSettings.h"

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

TEST(WeaponCatalogTests, FrameIndexFollowsWeaponIdOrder)
{
	for (WeaponId id : ALL_WEAPONS)
	{
		EXPECT_EQ(WeaponFrameIndex(id, 0), static_cast<int>(id) * WEAPON_VARIANTS) << "weapon " << GetWeapon(id).id;
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

TEST(WeaponCatalogTests, ExtraPropertiesBelongToTheirWeapon)
{
	EXPECT_EQ(FindMelee(WeaponId::Knife), &KNIFE_MELEE);
	EXPECT_EQ(FindMelee(WeaponId::Bat), &BAT_MELEE);
	EXPECT_EQ(FindSpread(WeaponId::ShotgunDouble), &SHOTGUN_DOUBLE_SPREAD);
	EXPECT_EQ(FindSpread(WeaponId::ShotgunPump), &SHOTGUN_PUMP_SPREAD);
	EXPECT_EQ(FindExplosive(WeaponId::Rpg), &RPG_EXPLOSIVE);

	EXPECT_EQ(FindMelee(WeaponId::Ak47), nullptr);
	EXPECT_EQ(FindSpread(WeaponId::Ak47), nullptr);
	EXPECT_EQ(FindExplosive(WeaponId::Ak47), nullptr);
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

TEST(WeaponCatalogTest, WeaponIsFoundByItsStringId)
{
	RoguelikeGame::WeaponId id = RoguelikeGame::WeaponId::Knife;

	EXPECT_TRUE(RoguelikeGame::TryGetWeaponId("ak47", id));
	EXPECT_EQ(id, RoguelikeGame::WeaponId::Ak47);

	EXPECT_TRUE(RoguelikeGame::TryGetWeaponId("deagle", id));
	EXPECT_EQ(id, RoguelikeGame::WeaponId::Deagle);

	EXPECT_FALSE(RoguelikeGame::TryGetWeaponId("railgun", id));
	EXPECT_FALSE(RoguelikeGame::TryGetWeaponId("", id));
}

TEST(WeaponCatalogTest, EveryWeaponIsReachableByItsId)
{
	for (int index = 0; index < RoguelikeGame::WEAPON_COUNT; index++)
	{
		auto expected = static_cast<RoguelikeGame::WeaponId>(index);
		RoguelikeGame::WeaponId found = RoguelikeGame::WeaponId::Knife;

		EXPECT_TRUE(RoguelikeGame::TryGetWeaponId(RoguelikeGame::WEAPONS[index].id, found));
		EXPECT_EQ(found, expected);
	}
}

TEST(WeaponCatalogTest, AmmoKindIsFoundByName)
{
	RoguelikeGame::AmmoKind kind = RoguelikeGame::AmmoKind::None;

	EXPECT_TRUE(RoguelikeGame::TryGetAmmoKind("pistol", kind));
	EXPECT_EQ(kind, RoguelikeGame::AmmoKind::Pistol);

	EXPECT_TRUE(RoguelikeGame::TryGetAmmoKind("rocket", kind));
	EXPECT_EQ(kind, RoguelikeGame::AmmoKind::Rocket);

	EXPECT_FALSE(RoguelikeGame::TryGetAmmoKind("plasma", kind));
}

TEST(WeaponCatalogTest, WeaponKnowsItsSlot)
{
	EXPECT_EQ(RoguelikeGame::PreferredWeaponSlot(RoguelikeGame::WeaponId::Ak47), 0);
	EXPECT_EQ(RoguelikeGame::PreferredWeaponSlot(RoguelikeGame::WeaponId::Rpg), 0);
	EXPECT_EQ(RoguelikeGame::PreferredWeaponSlot(RoguelikeGame::WeaponId::Glock), 1);
	EXPECT_EQ(RoguelikeGame::PreferredWeaponSlot(RoguelikeGame::WeaponId::ShotgunPump), 1);
	EXPECT_EQ(RoguelikeGame::PreferredWeaponSlot(RoguelikeGame::WeaponId::Knife), RoguelikeGame::PLAYER_WEAPON_SLOTS - 1);
	EXPECT_EQ(RoguelikeGame::PreferredWeaponSlot(RoguelikeGame::WeaponId::Bat), RoguelikeGame::PLAYER_WEAPON_SLOTS - 1);
}

TEST(WeaponCatalogTest, PlayerStartsWithMeleeOnly)
{
	int armed = 0;
	for (const RoguelikeGame::StartingSlot& slot : RoguelikeGame::PLAYER_LOADOUT)
	{
		if (slot.hasWeapon)
		{
			armed++;
			EXPECT_TRUE(RoguelikeGame::IsMelee(slot.id));
		}
	}

	EXPECT_EQ(armed, 1);
	EXPECT_TRUE(RoguelikeGame::PLAYER_LOADOUT[RoguelikeGame::PLAYER_START_WEAPON_SLOT].hasWeapon);

	for (const RoguelikeGame::AmmoReserve& reserve : RoguelikeGame::PLAYER_START_AMMO)
	{
		EXPECT_EQ(reserve.count, 0);
	}
}
