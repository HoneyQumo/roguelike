#include "pch.h"
#include "GameWorld.h"
#include "InventoryComponent.h"

using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemType;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	ItemDefinition MakePotion()
	{
		ItemDefinition item;
		item.id = "potion_small";
		item.name = "Potion";
		item.type = ItemType::Consumable;
		item.stackable = true;
		item.maxStack = 5;
		return item;
	}

	ItemDefinition MakeKey()
	{
		ItemDefinition item;
		item.id = "key_rusty";
		item.name = "Key";
		item.type = ItemType::Key;
		item.stackable = false;
		item.maxStack = 1;
		return item;
	}

	class InventoryTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		InventoryComponent* CreateInventory(int capacity)
		{
			GameObject* owner = GameWorld::Instance()->CreateGameObject("Player");
			auto inventory = owner->AddComponent<InventoryComponent>();
			inventory->SetCapacity(capacity);

			return inventory;
		}
	};
}

TEST_F(InventoryTest, NewInventoryIsEmpty)
{
	InventoryComponent* inventory = CreateInventory(6);

	EXPECT_EQ(inventory->GetCapacity(), 6);
	EXPECT_EQ(inventory->GetUsedSlots(), 0);
	EXPECT_FALSE(inventory->IsFull());
	EXPECT_TRUE(inventory->GetSlot(0).IsEmpty());
}

TEST_F(InventoryTest, SameItemsStackIntoOneSlot)
{
	InventoryComponent* inventory = CreateInventory(6);
	ItemDefinition potion = MakePotion();

	for (int taken = 0; taken < 5; taken++)
	{
		EXPECT_TRUE(inventory->TryAdd(potion));
	}

	EXPECT_EQ(inventory->GetUsedSlots(), 1);
	EXPECT_EQ(inventory->CountOf("potion_small"), 5);
	EXPECT_EQ(inventory->GetSlot(0).count, 5);
}

TEST_F(InventoryTest, StackOverflowGoesToTheNextSlot)
{
	InventoryComponent* inventory = CreateInventory(6);
	ItemDefinition potion = MakePotion();

	inventory->TryAdd(potion, 7);

	EXPECT_EQ(inventory->GetUsedSlots(), 2);
	EXPECT_EQ(inventory->GetSlot(0).count, 5);
	EXPECT_EQ(inventory->GetSlot(1).count, 2);
	EXPECT_EQ(inventory->CountOf("potion_small"), 7);
}

TEST_F(InventoryTest, NonStackableItemsTakeSeparateSlots)
{
	InventoryComponent* inventory = CreateInventory(6);
	ItemDefinition key = MakeKey();

	inventory->TryAdd(key);
	inventory->TryAdd(key);

	EXPECT_EQ(inventory->GetUsedSlots(), 2);
	EXPECT_EQ(inventory->CountOf("key_rusty"), 2);
}

TEST_F(InventoryTest, FullInventoryRefusesTheItemAndTellsAboutIt)
{
	InventoryComponent* inventory = CreateInventory(2);
	ItemDefinition key = MakeKey();
	inventory->TryAdd(key);
	inventory->TryAdd(key);

	int rejections = 0;
	std::string rejectedId;
	inventory->SubscribeRejected([&rejections, &rejectedId](const ItemDefinition& item)
	{
		rejections++;
		rejectedId = item.id;
	});

	EXPECT_FALSE(inventory->TryAdd(key));

	EXPECT_TRUE(inventory->IsFull());
	EXPECT_EQ(inventory->CountOf("key_rusty"), 2);
	EXPECT_EQ(rejections, 1);
	EXPECT_EQ(rejectedId, "key_rusty");
}

TEST_F(InventoryTest, PartialFitIsRefusedWholly)
{
	InventoryComponent* inventory = CreateInventory(1);
	ItemDefinition potion = MakePotion();

	EXPECT_FALSE(inventory->TryAdd(potion, 7));

	EXPECT_EQ(inventory->GetUsedSlots(), 0);
	EXPECT_EQ(inventory->CountOf("potion_small"), 0);
}

TEST_F(InventoryTest, LastItemOfStackFreesTheSlot)
{
	InventoryComponent* inventory = CreateInventory(4);
	ItemDefinition potion = MakePotion();
	inventory->TryAdd(potion, 2);

	EXPECT_TRUE(inventory->Remove(0, 1));
	EXPECT_EQ(inventory->GetUsedSlots(), 1);

	EXPECT_TRUE(inventory->Remove(0, 1));

	EXPECT_EQ(inventory->GetUsedSlots(), 0);
	EXPECT_TRUE(inventory->GetSlot(0).IsEmpty());
	EXPECT_FALSE(inventory->Contains("potion_small"));
}

TEST_F(InventoryTest, UsingEmptySlotIsSafeRefusal)
{
	InventoryComponent* inventory = CreateInventory(4);
	int used = 0;
	inventory->SubscribeUsed([&used](const ItemDefinition&) { used++; });

	EXPECT_FALSE(inventory->Use(0));
	EXPECT_FALSE(inventory->Use(99));
	EXPECT_FALSE(inventory->Use(-1));

	EXPECT_EQ(used, 0);
}

TEST_F(InventoryTest, UsingItemSpendsOneAndReportsIt)
{
	InventoryComponent* inventory = CreateInventory(4);
	ItemDefinition potion = MakePotion();
	inventory->TryAdd(potion, 3);

	std::string usedId;
	inventory->SubscribeUsed([&usedId](const ItemDefinition& item) { usedId = item.id; });

	EXPECT_TRUE(inventory->Use(0));

	EXPECT_EQ(usedId, "potion_small");
	EXPECT_EQ(inventory->CountOf("potion_small"), 2);
}

TEST_F(InventoryTest, SelectedSlotIsUsedByDefault)
{
	InventoryComponent* inventory = CreateInventory(4);
	ItemDefinition potion = MakePotion();
	ItemDefinition key = MakeKey();
	inventory->TryAdd(potion);
	inventory->TryAdd(key);

	inventory->SelectSlot(1);
	EXPECT_EQ(inventory->GetSelectedSlot(), 1);

	EXPECT_TRUE(inventory->UseSelected());
	EXPECT_FALSE(inventory->Contains("key_rusty"));
	EXPECT_TRUE(inventory->Contains("potion_small"));
}

TEST_F(InventoryTest, SelectingSlotOutOfRangeKeepsTheOldOne)
{
	InventoryComponent* inventory = CreateInventory(4);
	inventory->SelectSlot(2);

	inventory->SelectSlot(99);
	inventory->SelectSlot(-5);

	EXPECT_EQ(inventory->GetSelectedSlot(), 2);
}

TEST_F(InventoryTest, AddAndRemoveEventsCarryTheItemAndCount)
{
	InventoryComponent* inventory = CreateInventory(4);
	ItemDefinition potion = MakePotion();

	std::string addedId;
	int addedCount = 0;
	std::string removedId;
	int removedCount = 0;
	int changes = 0;

	inventory->SubscribeAdded([&addedId, &addedCount](const ItemDefinition& item, int count)
	{
		addedId = item.id;
		addedCount = count;
	});
	inventory->SubscribeRemoved([&removedId, &removedCount](const ItemDefinition& item, int count)
	{
		removedId = item.id;
		removedCount = count;
	});
	inventory->SubscribeChanged([&changes]() { changes++; });

	inventory->TryAdd(potion, 3);
	inventory->Remove(0, 2);

	EXPECT_EQ(addedId, "potion_small");
	EXPECT_EQ(addedCount, 3);
	EXPECT_EQ(removedId, "potion_small");
	EXPECT_EQ(removedCount, 2);
	EXPECT_EQ(changes, 2);
}

TEST_F(InventoryTest, RemovingMoreThanStoredIsRefused)
{
	InventoryComponent* inventory = CreateInventory(4);
	ItemDefinition potion = MakePotion();
	inventory->TryAdd(potion, 2);

	EXPECT_FALSE(inventory->Remove(0, 3));
	EXPECT_EQ(inventory->CountOf("potion_small"), 2);
}

TEST_F(InventoryTest, StacksFillGapsBeforeTakingNewSlots)
{
	InventoryComponent* inventory = CreateInventory(4);
	ItemDefinition potion = MakePotion();
	ItemDefinition key = MakeKey();

	inventory->TryAdd(potion, 3);
	inventory->TryAdd(key);
	inventory->TryAdd(potion, 2);

	EXPECT_EQ(inventory->GetUsedSlots(), 2);
	EXPECT_EQ(inventory->GetSlot(0).count, 5);
	EXPECT_TRUE(inventory->GetSlot(1).Holds("key_rusty"));
}
