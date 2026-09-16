#include "pch.h"
#include "GameSettings.h"
#include "WeaponCatalog.h"

using RoguelikeGame::MovePaceOf;
using RoguelikeGame::WeaponId;
using RoguelikeGame::WeightOf;
using RoguelikeGame::WEAPONS;
using RoguelikeGame::WEAPON_COUNT;
using RoguelikeGame::WEAPON_WEIGHTS;

namespace
{
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

		EXPECT_GT(pace, 0.6f) << WEAPONS[index].id << " turns the carrier into a statue";
		EXPECT_LE(pace, 1.f) << WEAPONS[index].id << " makes the carrier faster than empty handed";
	}
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
