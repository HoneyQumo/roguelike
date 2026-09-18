#include "pch.h"
#include "LoadoutRules.h"

using RoguelikeGame::EquipOutcome;
using RoguelikeGame::LoadoutState;
using RoguelikeGame::StartingSlot;
using RoguelikeGame::WeaponId;

namespace
{
	constexpr int MELEE_SLOT = RoguelikeGame::PLAYER_WEAPON_SLOTS - 1;

	LoadoutState StartingLoadout()
	{
		LoadoutState state;
		state.Fill(RoguelikeGame::PLAYER_LOADOUT, RoguelikeGame::PLAYER_WEAPON_SLOTS);

		return state;
	}
}

TEST(LoadoutRulesTest, TheRunStartsWithAKnifeAndTwoEmptyHands)
{
	LoadoutState state = StartingLoadout();

	EXPECT_EQ(state.slotsCount, RoguelikeGame::PLAYER_WEAPON_SLOTS);
	EXPECT_TRUE(state.IsEmpty(0));
	EXPECT_TRUE(state.IsEmpty(1));
	EXPECT_FALSE(state.IsEmpty(MELEE_SLOT));
	EXPECT_EQ(state.slots[MELEE_SLOT].id, WeaponId::Knife);
}

TEST(LoadoutRulesTest, AnEmptyStartingSlotHandsOverToTheFirstArmedOne)
{
	LoadoutState state = StartingLoadout();

	EXPECT_EQ(state.FirstArmed(0), MELEE_SLOT) << "забег начинается с пустыми руками";
	EXPECT_EQ(state.FirstArmed(MELEE_SLOT), MELEE_SLOT);
}

TEST(LoadoutRulesTest, AWeaponGoesToTheSlotOfItsKind)
{
	LoadoutState state = StartingLoadout();

	EXPECT_EQ(state.ResolveSlot(WeaponId::Ak47), 0);
	EXPECT_EQ(state.ResolveSlot(WeaponId::Deagle), 1);
	EXPECT_EQ(state.ResolveSlot(WeaponId::Knife), MELEE_SLOT);
}

TEST(LoadoutRulesTest, AFreeSlotTakesTheWeaponAndAnOccupiedOneDoesNot)
{
	LoadoutState state = StartingLoadout();

	EXPECT_TRUE(state.CanTake(WeaponId::Deagle));
	EXPECT_FALSE(state.CanTake(WeaponId::Knife)) << "слот ближнего боя занят ножом";

	state.Equip(WeaponId::Deagle);

	EXPECT_FALSE(state.CanTake(WeaponId::Glock)) << "тот же слот, а он уже занят";
}

TEST(LoadoutRulesTest, AnEmptySlotIsNotWorthSwitchingTo)
{
	LoadoutState state = StartingLoadout();
	state.currentSlot = MELEE_SLOT;

	EXPECT_FALSE(state.CanSelect(0)) << "переключились на пустые руки";
	EXPECT_FALSE(state.CanSelect(MELEE_SLOT)) << "переключились сами на себя";
	EXPECT_FALSE(state.CanSelect(RoguelikeGame::NO_WEAPON_SLOT));
	EXPECT_FALSE(state.CanSelect(RoguelikeGame::PLAYER_WEAPON_SLOTS));

	state.Equip(WeaponId::Deagle);

	EXPECT_TRUE(state.CanSelect(1));
}

TEST(LoadoutRulesTest, EquippingTheSameWeaponTwiceChangesNothing)
{
	LoadoutState state = StartingLoadout();

	ASSERT_TRUE(state.Equip(WeaponId::Deagle).isChanged);

	EquipOutcome again = state.Equip(WeaponId::Deagle);

	EXPECT_FALSE(again.isChanged);
	EXPECT_EQ(again.slot, RoguelikeGame::NO_WEAPON_SLOT);
}

TEST(LoadoutRulesTest, AWeaponArrivesWithAFullMagazine)
{
	LoadoutState state = StartingLoadout();

	EquipOutcome equipped = state.Equip(WeaponId::Deagle);

	ASSERT_TRUE(equipped.isChanged);
	EXPECT_EQ(state.slots[equipped.slot].magazine, RoguelikeGame::GetWeapon(WeaponId::Deagle).magazineSize);
}

// Вытеснить оружие умеет только обмен - он знает, куда деть вытесненное.
TEST(LoadoutRulesTest, AnOccupiedSlotIsNotOverwritten)
{
	LoadoutState state = StartingLoadout();
	state.Equip(WeaponId::Deagle);
	ASSERT_EQ(state.slots[1].id, WeaponId::Deagle);

	EquipOutcome equipped = state.Equip(WeaponId::Glock);

	EXPECT_FALSE(equipped.isChanged) << "дигл пропал без следа";
	EXPECT_EQ(state.slots[1].id, WeaponId::Deagle);
}

TEST(LoadoutRulesTest, MeleeBelongsToTheLastSlotAndFirearmsToTheRest)
{
	LoadoutState state = StartingLoadout();

	EXPECT_TRUE(state.Fits(MELEE_SLOT, WeaponId::Knife));
	EXPECT_FALSE(state.Fits(MELEE_SLOT, WeaponId::Deagle)) << "огнестрел занял слот ближнего боя";
	EXPECT_TRUE(state.Fits(0, WeaponId::Ak47));
	EXPECT_FALSE(state.Fits(0, WeaponId::Bat)) << "бита уехала в оружейный слот";
	EXPECT_FALSE(state.Fits(RoguelikeGame::NO_WEAPON_SLOT, WeaponId::Deagle));
	EXPECT_FALSE(state.Fits(state.slotsCount, WeaponId::Deagle));
}

TEST(LoadoutRulesTest, AnEmptyLoadoutAnswersWithoutCountingSlots)
{
	LoadoutState state;

	EXPECT_EQ(state.slotsCount, 0);
	EXPECT_EQ(state.ResolveSlot(WeaponId::Deagle), RoguelikeGame::NO_WEAPON_SLOT);
	EXPECT_EQ(state.FirstArmed(0), RoguelikeGame::NO_WEAPON_SLOT);
	EXPECT_FALSE(state.CanTake(WeaponId::Deagle));
	EXPECT_FALSE(state.Equip(WeaponId::Deagle).isChanged);
	EXPECT_TRUE(state.IsEmpty(0));
}

TEST(LoadoutRulesTest, MoreStartingSlotsThanThereIsRoomForAreCutOff)
{
	constexpr StartingSlot TOO_MANY[] = {
		{true, WeaponId::Ak47},
		{true, WeaponId::Glock},
		{true, WeaponId::Knife},
		{true, WeaponId::Deagle}
	};

	LoadoutState state;
	state.Fill(TOO_MANY, static_cast<int>(std::size(TOO_MANY)));

	EXPECT_EQ(state.slotsCount, RoguelikeGame::PLAYER_WEAPON_SLOTS);
}
