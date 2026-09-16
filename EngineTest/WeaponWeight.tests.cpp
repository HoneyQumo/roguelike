#include "pch.h"
#include "GameSettings.h"
#include "WeaponCatalog.h"

using RoguelikeGame::MovePaceOf;
using RoguelikeGame::SLOWEST_PACE;
using RoguelikeGame::WeaponId;
using RoguelikeGame::WeightOf;
using RoguelikeGame::WEAPONS;
using RoguelikeGame::WEAPON_COUNT;
using RoguelikeGame::WEAPON_WEIGHTS;

namespace
{
	// Насколько шаг обязан просесть от класса к классу, чтобы разницу было видно без секундомера.
	constexpr float CLASS_GAP = 0.045f;

	// По одному представителю на класс, от самого лёгкого к самому тяжёлому.
	constexpr WeaponId WEAPON_CLASSES[] = {
		WeaponId::Knife,
		WeaponId::Glock,
		WeaponId::SmgSuppressed,
		WeaponId::Ak47,
		WeaponId::Rpg
	};

	float WalkSpeedWith(WeaponId id)
	{
		return RoguelikeGame::PLAYER_SPEED * MovePaceOf(id);
	}
}

TEST(WeaponWeightTests, EveryWeaponIsWeighed)
{
	for (int index = 0; index < WEAPON_COUNT; index++)
	{
		WeaponId id = static_cast<WeaponId>(index);

		EXPECT_GT(WeightOf(id), 0.f) << WEAPONS[index].id << " weighs nothing";
	}
}

TEST(WeaponWeightTests, NoWeaponIsWeighedTwice)
{
	for (std::size_t first = 0u; first < std::size(WEAPON_WEIGHTS); first++)
	{
		for (std::size_t second = first + 1u; second < std::size(WEAPON_WEIGHTS); second++)
		{
			EXPECT_NE(WEAPON_WEIGHTS[first].weapon, WEAPON_WEIGHTS[second].weapon);
		}
	}
}

TEST(WeaponWeightTests, TheLightestWeaponDoesNotSlowTheWalkAtAll)
{
	EXPECT_FLOAT_EQ(MovePaceOf(WeaponId::Knife), 1.f);
}

TEST(WeaponWeightTests, HeavierIronMeansAShorterStep)
{
	EXPECT_GT(MovePaceOf(WeaponId::Knife), MovePaceOf(WeaponId::Glock)) << "a pistol costs as little as bare hands";
	EXPECT_GT(MovePaceOf(WeaponId::Glock), MovePaceOf(WeaponId::SmgSuppressed));
	EXPECT_GT(MovePaceOf(WeaponId::SmgSuppressed), MovePaceOf(WeaponId::M16));
	EXPECT_GT(MovePaceOf(WeaponId::M16), MovePaceOf(WeaponId::Ak47)) << "the rifle order is upside down";
	EXPECT_GT(MovePaceOf(WeaponId::Ak47), MovePaceOf(WeaponId::Rpg));
}

TEST(WeaponWeightTests, EvenTheHeaviestWeaponLeavesTheCarrierMoving)
{
	for (int index = 0; index < WEAPON_COUNT; index++)
	{
		float pace = MovePaceOf(static_cast<WeaponId>(index));

		EXPECT_GE(pace, SLOWEST_PACE) << WEAPONS[index].id << " turns the carrier into a statue";
		EXPECT_LE(pace, 1.f) << WEAPONS[index].id << " makes the carrier faster than empty handed";
	}
}

TEST(WeaponWeightTests, EveryClassIsClearlySlowerThanTheOneBefore)
{
	for (std::size_t index = 1u; index < std::size(WEAPON_CLASSES); index++)
	{
		float lighter = MovePaceOf(WEAPON_CLASSES[index - 1u]);
		float heavier = MovePaceOf(WEAPON_CLASSES[index]);

		EXPECT_GE(lighter - heavier, CLASS_GAP)
			<< WEAPONS[static_cast<int>(WEAPON_CLASSES[index])].id << " feels the same as "
			<< WEAPONS[static_cast<int>(WEAPON_CLASSES[index - 1u])].id;
	}
}

TEST(WeaponWeightTests, TheHeaviestClassIsAWorldApartFromTheLightest)
{
	float spread = MovePaceOf(WeaponId::Knife) - MovePaceOf(WeaponId::Rpg);

	EXPECT_GE(spread, 0.3f) << "the whole range from a knife to a rocket is not worth a choice";
}

TEST(WeaponWeightTests, WhatShootsAlsoCarriesWhatItShoots)
{
	for (int index = 0; index < WEAPON_COUNT; index++)
	{
		WeaponId id = static_cast<WeaponId>(index);
		const RoguelikeGame::WeaponWeight* entry = RoguelikeGame::FindWeight(id);

		ASSERT_NE(entry, nullptr) << WEAPONS[index].id;

		if (RoguelikeGame::IsMelee(id))
		{
			EXPECT_FLOAT_EQ(entry->ammoKilograms, 0.f) << WEAPONS[index].id << " carries ammo it never fires";
			continue;
		}

		EXPECT_GT(entry->ammoKilograms, 0.f) << WEAPONS[index].id << " shoots out of thin air";
	}
}

TEST(WeaponWeightTests, AmmoIsAWeightWorthCounting)
{
	const RoguelikeGame::WeaponWeight* rifle = RoguelikeGame::FindWeight(WeaponId::Ak47);

	ASSERT_NE(rifle, nullptr);

	EXPECT_GT(rifle->ammoKilograms, 0.25f * rifle->kilograms) << "the load of magazines rounds down to nothing";
}

TEST(WeaponWeightTests, TheDifferenceIsWorthFeeling)
{
	float gap = WalkSpeedWith(WeaponId::Knife) - WalkSpeedWith(WeaponId::Ak47);

	EXPECT_GT(gap, 20.f) << "nobody will notice a rifle that costs less than a step";
}

TEST(WeaponWeightTests, ARunnerWithARifleStillOutrunsAWalkerWithAKnife)
{
	float runWithRifle = WalkSpeedWith(WeaponId::Ak47) * RoguelikeGame::PLAYER_RUN_SPEED_MULTIPLIER;

	EXPECT_GT(runWithRifle, WalkSpeedWith(WeaponId::Knife)) << "running became pointless";
}
