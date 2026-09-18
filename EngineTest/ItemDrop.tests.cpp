#include "pch.h"
#include "GameSettings.h"
#include "ItemDropComponent.h"
#include <GameWorld.h>
#include <InputSystem.h>

using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemDropComponent;
using RoguelikeGame::ItemEffectKind;
using RoguelikeGame::ItemType;
using RoguelikeGame::NO_CHARGE;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::InputAction;
using XYZEngine::Vector2Df;

namespace
{
	ItemDefinition Potion()
	{
		ItemDefinition item;
		item.id = "potion";
		item.name = "potion";
		item.type = ItemType::Consumable;
		item.stackable = true;
		item.maxStack = 5;
		item.effect.kind = ItemEffectKind::Heal;

		return item;
	}

	ItemDefinition Rifle()
	{
		ItemDefinition item;
		item.id = "weapon_ak";
		item.name = "AK";
		item.type = ItemType::Weapon;
		item.stackable = false;
		item.maxStack = 1;
		item.effect.kind = ItemEffectKind::EquipWeapon;

		return item;
	}

	struct SpawnRecord
	{
		std::string id;
		int count = 0;
		int charge = NO_CHARGE;
		Vector2Df place;
	};

	class ItemDropTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			owner = GameWorld::Instance()->CreateGameObject("Player");
			owner->GetTransform()->SetWorldPosition({100.f, 200.f});

			bag = owner->AddComponent<InventoryComponent>();
			bag->SetCapacity(8);

			drop = owner->AddComponent<ItemDropComponent>();
			drop->SetInventory(bag);
			AllowSpawn(true);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		void AllowSpawn(bool isAllowed)
		{
			drop->SetSpawner([this, isAllowed](const ItemDefinition& item, int count, int charge, const Vector2Df& place)
			{
				spawned.push_back({item.id, count, charge, place});

				return isAllowed;
			});
		}

		GameObject* owner = nullptr;
		InventoryComponent* bag = nullptr;
		ItemDropComponent* drop = nullptr;
		std::vector<SpawnRecord> spawned;
	};
}

TEST_F(ItemDropTest, TheWholeStackLeavesTheBagAtOnce)
{
	bag->TryAdd(Potion(), 3);

	EXPECT_TRUE(drop->Drop(0));

	ASSERT_EQ(spawned.size(), 1u);
	EXPECT_EQ(spawned[0].id, "potion");
	EXPECT_EQ(spawned[0].count, 3);
	EXPECT_TRUE(bag->GetSlot(0).IsEmpty());
}

TEST_F(ItemDropTest, ADroppedWeaponCarriesItsMagazine)
{
	bag->Replace(0, Rifle(), 7);

	EXPECT_TRUE(drop->Drop(0));

	ASSERT_EQ(spawned.size(), 1u);
	EXPECT_EQ(spawned[0].charge, 7);
}

TEST_F(ItemDropTest, APickedUpWeaponRemembersItsMagazine)
{
	EXPECT_TRUE(bag->TryAdd(Rifle(), 1, 7));

	EXPECT_EQ(bag->GetSlot(0).charge, 7);
}

TEST_F(ItemDropTest, AStackNeverCarriesAMagazine)
{
	EXPECT_TRUE(bag->TryAdd(Potion(), 2, 7));

	EXPECT_EQ(bag->GetSlot(0).charge, NO_CHARGE);
}

TEST_F(ItemDropTest, AnEmptyCellDropsNothing)
{
	EXPECT_FALSE(drop->Drop(0));

	EXPECT_TRUE(spawned.empty());
}

TEST_F(ItemDropTest, ACellOutsideTheBagDropsNothing)
{
	EXPECT_FALSE(drop->Drop(-1));
	EXPECT_FALSE(drop->Drop(bag->GetCapacity()));

	EXPECT_TRUE(spawned.empty());
}

// Иначе предмет исчезает: из сумки убран, на полу не родился.
TEST_F(ItemDropTest, WhenTheFloorRefusesTheStackStaysInTheBag)
{
	bag->TryAdd(Potion(), 3);
	AllowSpawn(false);

	EXPECT_FALSE(drop->Drop(0));

	EXPECT_EQ(bag->GetSlot(0).count, 3);
}

TEST_F(ItemDropTest, AFrozenPlayerDropsNothing)
{
	bag->TryAdd(Potion(), 1);
	bag->SetEnabled(false);

	EXPECT_FALSE(drop->Drop(0));

	EXPECT_TRUE(spawned.empty());
	bag->SetEnabled(true);
	EXPECT_TRUE(drop->Drop(0));
}

// Слой предмета ниже слоя игрока: брошенное точно под ноги пропадёт под спрайтом.
TEST_F(ItemDropTest, TheStackLandsAStepAwayFromTheFeet)
{
	bag->TryAdd(Potion(), 1);

	EXPECT_TRUE(drop->Drop(0));

	ASSERT_EQ(spawned.size(), 1u);
	EXPECT_FLOAT_EQ(spawned[0].place.x, 100.f);
	EXPECT_FLOAT_EQ(spawned[0].place.y, 200.f - RoguelikeGame::DROP_STEP);
}

TEST_F(ItemDropTest, WithoutASpawnerNothingLeavesTheBag)
{
	bag->TryAdd(Potion(), 2);
	drop->SetSpawner(nullptr);

	EXPECT_FALSE(drop->Drop(0));

	EXPECT_EQ(bag->GetSlot(0).count, 2);
}

// Выбор слота оружия и крючка - арифметика от первого действия: вставка в середину ломает её молча.
TEST(ItemDropInput, TheDropActionSitsAtTheTail)
{
	EXPECT_EQ(static_cast<int>(InputAction::Drop), static_cast<int>(InputAction::Count) - 1);
	EXPECT_EQ(XYZEngine::GetDefaultBindings()[static_cast<int>(InputAction::Drop)].key, sf::Keyboard::G);
}
