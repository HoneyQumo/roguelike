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
	EXPECT_FLOAT_EQ(profile.coneDegrees, SpreadOf(WeaponId::Ak47));
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

namespace
{
	float DamagePerSecond(WeaponId id)
	{
		const FireProfile* fire = FindFire(id);

		return fire == nullptr ? 0.f : fire->damage / fire->cooldown;
	}
}

TEST(WeaponFireTests, EveryFiringArmIsInTheFireTable)
{
	for (WeaponId id : ALL_WEAPONS)
	{
		if (IsMelee(id) || FindSpread(id) != nullptr || IsExplosive(id))
		{
			continue;
		}

		EXPECT_NE(FindFire(id), nullptr) << GetWeapon(id).id << " has no line in the fire table";
	}
}

TEST(WeaponFireTests, TheAkHitsHarderThanTheM16AndSpraysWider)
{
	const FireProfile* ak = FindFire(WeaponId::Ak47);
	const FireProfile* m16 = FindFire(WeaponId::M16);
	ASSERT_NE(ak, nullptr);
	ASSERT_NE(m16, nullptr);

	EXPECT_GT(ak->damage, m16->damage);
	EXPECT_GT(ak->spreadDegrees, m16->spreadDegrees);
}

TEST(WeaponFireTests, TheM16OutrunsTheAk)
{
	EXPECT_LT(FindFire(WeaponId::M16)->cooldown, FindFire(WeaponId::Ak47)->cooldown);
}

TEST(WeaponFireTests, TheSmgIsTheFastestAndTheWeakestPerShot)
{
	const FireProfile* smg = FindFire(WeaponId::SmgSuppressed);
	ASSERT_NE(smg, nullptr);

	for (const FireProfile& other : WEAPON_FIRE)
	{
		if (other.weapon == WeaponId::SmgSuppressed)
		{
			continue;
		}

		EXPECT_LT(smg->cooldown, other.cooldown) << GetWeapon(other.weapon).id << " fires faster than the smg";
	}

	EXPECT_LT(smg->damage, FindFire(WeaponId::Ak47)->damage);
	EXPECT_LT(smg->damage, FindFire(WeaponId::M16)->damage);
}

TEST(WeaponFireTests, TheDeagleIsTheSlowestAndTheHardestHitter)
{
	const FireProfile* deagle = FindFire(WeaponId::Deagle);
	ASSERT_NE(deagle, nullptr);

	for (const FireProfile& other : WEAPON_FIRE)
	{
		if (other.weapon == WeaponId::Deagle)
		{
			continue;
		}

		EXPECT_GT(deagle->cooldown, other.cooldown) << GetWeapon(other.weapon).id << " is slower than the deagle";
		EXPECT_GT(deagle->damage, other.damage) << GetWeapon(other.weapon).id << " hits harder than the deagle";
	}
}

TEST(WeaponFireTests, RiflesAndTheSmgFireFasterThanThePlayerUsedTo)
{
	for (WeaponId id : {WeaponId::Ak47, WeaponId::M16, WeaponId::SmgSuppressed})
	{
		EXPECT_LT(FindFire(id)->cooldown, PLAYER_ATTACK_COOLDOWN) << GetWeapon(id).id << " is not faster than the old pace";
	}
}

TEST(WeaponFireTests, NoArmRunsAwayWithTheDamagePerSecond)
{
	for (const FireProfile& fire : WEAPON_FIRE)
	{
		float dps = DamagePerSecond(fire.weapon);

		EXPECT_GT(dps, 60.f) << GetWeapon(fire.weapon).id << " is not worth carrying";
		EXPECT_LT(dps, 250.f) << GetWeapon(fire.weapon).id << " melts everything";
	}
}

TEST(WeaponFireTests, AMagazineLastsLongEnoughToBeFelt)
{
	for (const FireProfile& fire : WEAPON_FIRE)
	{
		float emptyIn = GetWeapon(fire.weapon).magazineSize * fire.cooldown;

		EXPECT_GT(emptyIn, 1.5f) << GetWeapon(fire.weapon).id << " empties before the player notices";
	}
}

TEST(WeaponFireTests, TheOwnerOfTheArmShootsWithItsOwnNumbers)
{
	ShotProfile shot = MakeWeaponShotProfile(WeaponId::Ak47, 100.f, 800.f, 0.5f);
	const FireProfile* ak = FindFire(WeaponId::Ak47);

	EXPECT_FLOAT_EQ(shot.damage, ak->damage);
	EXPECT_FLOAT_EQ(shot.cooldown, ak->cooldown);
	EXPECT_FLOAT_EQ(shot.speed, ak->speed);
	EXPECT_FLOAT_EQ(shot.coneDegrees, ak->spreadDegrees);
}

TEST(WeaponFireTests, ABorrowedArmKeepsTheShooterPowerButTakesTheSpread)
{
	ShotProfile shot = MakeShotProfile(WeaponId::Ak47, 100.f, 800.f, 0.5f);

	EXPECT_FLOAT_EQ(shot.damage, 100.f);
	EXPECT_FLOAT_EQ(shot.cooldown, 0.5f);
	EXPECT_FLOAT_EQ(shot.coneDegrees, FindFire(WeaponId::Ak47)->spreadDegrees);
}

TEST(WeaponFireTests, AnArmOutsideTheTableFallsBackToTheShooter)
{
	ShotProfile shot = MakeWeaponShotProfile(WeaponId::ShotgunPump, 100.f, 800.f, 0.5f);
	ShotProfile plain = MakeShotProfile(WeaponId::ShotgunPump, 100.f, 800.f, 0.5f);

	EXPECT_FLOAT_EQ(shot.damage, plain.damage);
	EXPECT_FLOAT_EQ(shot.cooldown, plain.cooldown);
}
