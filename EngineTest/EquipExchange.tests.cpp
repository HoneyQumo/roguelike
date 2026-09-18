#include "pch.h"
#include "EquipExchange.h"
#include "ItemCatalogLoader.h"
#include <GameWorld.h>
#include <sstream>

using RoguelikeGame::EquipRefusal;
using RoguelikeGame::EquipResult;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemCatalog;
using RoguelikeGame::ItemCatalogLoader;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::LoadoutState;
using RoguelikeGame::NO_CHARGE;
using RoguelikeGame::NO_WEAPON_SLOT;
using RoguelikeGame::TryEquipFromBag;
using RoguelikeGame::WeaponId;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	constexpr int MELEE_SLOT = RoguelikeGame::PLAYER_WEAPON_SLOTS - 1;

	// Ровно те записи каталога, что нужны обмену: ствол превращается в предмет по эффекту.
	const std::string CATALOG =
		"[item weapon_ak47]\n"
		"name AK\n"
		"type Weapon\n"
		"icon Resources/Textures/weapons_side.png 0 0 112 32\n"
		"effect EquipWeapon 0 ak47\n"
		"\n"
		"[item weapon_glock]\n"
		"name Glock\n"
		"type Weapon\n"
		"icon Resources/Textures/weapons_side.png 0 160 112 32\n"
		"effect EquipWeapon 0 glock\n"
		"\n"
		"[item weapon_deagle]\n"
		"name Deagle\n"
		"type Weapon\n"
		"icon Resources/Textures/weapons_side.png 0 192 112 32\n"
		"effect EquipWeapon 0 deagle\n"
		"\n"
		"[item weapon_bat]\n"
		"name Bat\n"
		"type Weapon\n"
		"icon Resources/Textures/weapons_side.png 0 288 112 32\n"
		"effect EquipWeapon 0 bat\n"
		"\n"
		"[item weapon_knife]\n"
		"name Knife\n"
		"type Weapon\n"
		"icon Resources/Textures/weapons_side.png 0 256 112 32\n"
		"effect EquipWeapon 0 knife\n"
		"\n"
		"[item potion_small]\n"
		"name Potion\n"
		"icon Resources/Textures/fx.png 0 224 24 24\n"
		"stackable true\n"
		"maxStack 5\n"
		"effect Heal 35\n";

	class EquipExchangeTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			GameObject* owner = GameWorld::Instance()->CreateGameObject("Player");
			bag = owner->AddComponent<InventoryComponent>();
			bag->SetCapacity(RoguelikeGame::INVENTORY_CAPACITY);

			std::istringstream input(CATALOG);
			catalog = ItemCatalogLoader::Parse(input, "test");

			loadout.Fill(RoguelikeGame::PLAYER_LOADOUT, RoguelikeGame::PLAYER_WEAPON_SLOTS);
			loadout.currentSlot = MELEE_SLOT;
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		const ItemDefinition& Item(const std::string& id) const
		{
			const ItemDefinition* found = catalog.Find(id);
			EXPECT_NE(found, nullptr) << id;

			return *found;
		}

		int Put(const std::string& id, int charge = NO_CHARGE)
		{
			EXPECT_TRUE(bag->TryAdd(Item(id)));
			int slot = bag->FindSlot(id);

			if (charge != NO_CHARGE)
			{
				bag->Replace(slot, Item(id), charge);
			}

			return slot;
		}

		int CountWeapons() const
		{
			int total = 0;

			for (int slot = 0; slot < bag->GetCapacity(); slot++)
			{
				const RoguelikeGame::InventorySlot& carried = bag->GetSlot(slot);
				total += !carried.IsEmpty() && carried.item.effect.kind == RoguelikeGame::ItemEffectKind::EquipWeapon
					? carried.count : 0;
			}

			for (int slot = 0; slot < loadout.slotsCount; slot++)
			{
				total += loadout.IsEmpty(slot) ? 0 : 1;
			}

			return total;
		}

		InventoryComponent* bag = nullptr;
		ItemCatalog catalog;
		LoadoutState loadout;
	};
}

TEST_F(EquipExchangeTest, AWeaponTakesAnEmptySlotAndLeavesTheBag)
{
	int bagSlot = Put("weapon_deagle");

	EquipResult result = TryEquipFromBag(*bag, bagSlot, loadout, NO_WEAPON_SLOT, catalog);

	EXPECT_TRUE(result.isDone);
	EXPECT_EQ(result.slot, 1);
	EXPECT_FALSE(result.hasDisplaced);
	EXPECT_EQ(loadout.slots[1].id, WeaponId::Deagle);
	EXPECT_TRUE(bag->GetSlot(bagSlot).IsEmpty()) << "предмет остался в сумке";
}

TEST_F(EquipExchangeTest, AnOccupiedSlotGivesItsWeaponBackIntoTheSameCell)
{
	int bagSlot = Put("weapon_deagle");
	ASSERT_TRUE(TryEquipFromBag(*bag, bagSlot, loadout, NO_WEAPON_SLOT, catalog).isDone);

	int before = CountWeapons();
	int secondSlot = Put("weapon_glock");

	EquipResult result = TryEquipFromBag(*bag, secondSlot, loadout, 1, catalog);

	EXPECT_TRUE(result.isDone);
	EXPECT_TRUE(result.hasDisplaced);
	EXPECT_EQ(result.displaced, WeaponId::Deagle);
	EXPECT_EQ(loadout.slots[1].id, WeaponId::Glock);
	EXPECT_EQ(bag->GetSlot(secondSlot).item.id, "weapon_deagle") << "вытесненный ствол лёг не в ту ячейку";
	EXPECT_EQ(CountWeapons(), before + 1) << "оружие пропало из мира";
}

TEST_F(EquipExchangeTest, TheMagazineTravelsWithTheWeapon)
{
	int bagSlot = Put("weapon_deagle");
	ASSERT_TRUE(TryEquipFromBag(*bag, bagSlot, loadout, NO_WEAPON_SLOT, catalog).isDone);

	// Дигл отстрелял почти весь магазин.
	loadout.slots[1].magazine = 2;

	int glockSlot = Put("weapon_glock");
	ASSERT_TRUE(TryEquipFromBag(*bag, glockSlot, loadout, 1, catalog).isDone);

	EXPECT_EQ(bag->GetSlot(glockSlot).charge, 2) << "патроны снятого ствола потерялись";

	EquipResult back = TryEquipFromBag(*bag, glockSlot, loadout, 1, catalog);

	ASSERT_TRUE(back.isDone);
	EXPECT_EQ(loadout.slots[1].id, WeaponId::Deagle);
	EXPECT_EQ(loadout.slots[1].magazine, 2) << "круговой обмен напечатал патроны";
}

TEST_F(EquipExchangeTest, AWeaponThatWasNeverHeldArrivesFull)
{
	int bagSlot = Put("weapon_deagle");

	ASSERT_EQ(bag->GetSlot(bagSlot).charge, NO_CHARGE);
	ASSERT_TRUE(TryEquipFromBag(*bag, bagSlot, loadout, NO_WEAPON_SLOT, catalog).isDone);

	EXPECT_EQ(loadout.slots[1].magazine, RoguelikeGame::GetWeapon(WeaponId::Deagle).magazineSize);
}

TEST_F(EquipExchangeTest, AFullBagDoesNotBlockTheExchange)
{
	int bagSlot = Put("weapon_deagle");
	ASSERT_TRUE(TryEquipFromBag(*bag, bagSlot, loadout, NO_WEAPON_SLOT, catalog).isDone);

	int glockSlot = Put("weapon_glock");
	while (!bag->IsFull())
	{
		ASSERT_TRUE(bag->TryAdd(Item("potion_small")));
	}

	ASSERT_TRUE(bag->IsFull());

	EquipResult result = TryEquipFromBag(*bag, glockSlot, loadout, 1, catalog);

	EXPECT_TRUE(result.isDone) << "обмен на месте требует свободного места";
	EXPECT_EQ(bag->GetSlot(glockSlot).item.id, "weapon_deagle");
}

TEST_F(EquipExchangeTest, AFirearmDoesNotFitTheMeleeSlot)
{
	int bagSlot = Put("weapon_deagle");

	EquipResult result = TryEquipFromBag(*bag, bagSlot, loadout, MELEE_SLOT, catalog);

	EXPECT_FALSE(result.isDone);
	EXPECT_EQ(result.refusal, EquipRefusal::WrongKind);
	EXPECT_EQ(loadout.slots[MELEE_SLOT].id, WeaponId::Knife) << "игрок остался без ближнего боя";
	EXPECT_EQ(bag->GetSlot(bagSlot).item.id, "weapon_deagle") << "отказ всё равно тронул сумку";
}

TEST_F(EquipExchangeTest, MeleeGoesOnlyIntoTheMeleeSlotAndSwapsTheKnife)
{
	int bagSlot = Put("weapon_bat");

	EXPECT_EQ(TryEquipFromBag(*bag, bagSlot, loadout, 0, catalog).refusal, EquipRefusal::WrongKind);

	EquipResult result = TryEquipFromBag(*bag, bagSlot, loadout, MELEE_SLOT, catalog);

	EXPECT_TRUE(result.isDone);
	EXPECT_EQ(result.displaced, WeaponId::Knife);
	EXPECT_EQ(loadout.slots[MELEE_SLOT].id, WeaponId::Bat);
	EXPECT_FALSE(loadout.IsEmpty(MELEE_SLOT)) << "слот ближнего боя опустел";
	EXPECT_EQ(bag->GetSlot(bagSlot).item.id, "weapon_knife") << "нож потерялся";
}

TEST_F(EquipExchangeTest, TheAskedSlotBeatsThePreferredOne)
{
	int bagSlot = Put("weapon_deagle");

	EquipResult result = TryEquipFromBag(*bag, bagSlot, loadout, 0, catalog);

	EXPECT_TRUE(result.isDone);
	EXPECT_EQ(result.slot, 0) << "слот назначения снова считает оружие, а не игрок";
	EXPECT_TRUE(loadout.IsEmpty(1));
}

TEST_F(EquipExchangeTest, WhatIsNotAWeaponIsNotEquipped)
{
	ASSERT_TRUE(bag->TryAdd(Item("potion_small")));

	EquipResult result = TryEquipFromBag(*bag, bag->FindSlot("potion_small"), loadout, NO_WEAPON_SLOT, catalog);

	EXPECT_FALSE(result.isDone);
	EXPECT_EQ(result.refusal, EquipRefusal::NotAWeapon);

	EquipResult empty = TryEquipFromBag(*bag, 11, loadout, NO_WEAPON_SLOT, catalog);

	EXPECT_EQ(empty.refusal, EquipRefusal::NotAWeapon);
}

TEST_F(EquipExchangeTest, TheSameWeaponIsNotEquippedTwice)
{
	int bagSlot = Put("weapon_deagle");
	ASSERT_TRUE(TryEquipFromBag(*bag, bagSlot, loadout, NO_WEAPON_SLOT, catalog).isDone);

	int secondSlot = Put("weapon_deagle");
	EquipResult result = TryEquipFromBag(*bag, secondSlot, loadout, 1, catalog);

	EXPECT_FALSE(result.isDone);
	EXPECT_EQ(result.refusal, EquipRefusal::AlreadyThere);
}

TEST_F(EquipExchangeTest, AChargedCellNeverJoinsAStack)
{
	ASSERT_TRUE(bag->TryAdd(Item("potion_small"), 2));

	int slot = bag->FindSlot("potion_small");
	bag->Replace(slot, Item("potion_small"), 3);

	ASSERT_TRUE(bag->TryAdd(Item("potion_small")));

	EXPECT_EQ(bag->GetSlot(slot).count, 1) << "заряженную ячейку добили стопкой";
	EXPECT_EQ(bag->CountOf("potion_small"), 2);
}
