#include "pch.h"
#include "DoorRules.h"

using RoguelikeGame::FindKeyFor;
using RoguelikeGame::IsKeyFor;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemEffectKind;

namespace
{
	ItemDefinition Key(const std::string& id, const std::string& target)
	{
		ItemDefinition item;
		item.id = id;
		item.effect.kind = ItemEffectKind::Unlock;
		item.effect.target = target;

		return item;
	}

	ItemDefinition Potion()
	{
		ItemDefinition item;
		item.id = "potion_small";
		item.effect.kind = ItemEffectKind::Heal;
		item.effect.amount = 30.f;

		return item;
	}
}

TEST(DoorRulesTest, KeyWithTheRightTargetFits)
{
	EXPECT_TRUE(IsKeyFor(Key("key_rusty", "door_exit"), "door_exit"));
}

TEST(DoorRulesTest, KeyForAnotherDoorDoesNotFit)
{
	EXPECT_FALSE(IsKeyFor(Key("key_rusty", "door_vault"), "door_exit"));
}

TEST(DoorRulesTest, PotionIsNotAKey)
{
	EXPECT_FALSE(IsKeyFor(Potion(), "door_exit"));
}

TEST(DoorRulesTest, DoorWithoutAnIdTakesNoKey)
{
	EXPECT_FALSE(IsKeyFor(Key("key_rusty", ""), ""));
}

TEST(DoorRulesTest, KeyIsFoundAmongOtherThings)
{
	ItemDefinition potion = Potion();
	ItemDefinition key = Key("key_rusty", "door_exit");

	EXPECT_EQ(FindKeyFor({&potion, &key}, "door_exit"), &key);
}

TEST(DoorRulesTest, EmptySlotsAreSkipped)
{
	ItemDefinition key = Key("key_rusty", "door_exit");

	EXPECT_EQ(FindKeyFor({nullptr, &key, nullptr}, "door_exit"), &key);
}

TEST(DoorRulesTest, WithoutAKeyNothingIsFound)
{
	ItemDefinition potion = Potion();

	EXPECT_EQ(FindKeyFor({&potion}, "door_exit"), nullptr);
	EXPECT_EQ(FindKeyFor({}, "door_exit"), nullptr);
}

TEST(DoorRulesTest, FirstFittingKeyWins)
{
	ItemDefinition first = Key("key_rusty", "door_exit");
	ItemDefinition second = Key("key_copy", "door_exit");

	EXPECT_EQ(FindKeyFor({&first, &second}, "door_exit"), &first);
}
