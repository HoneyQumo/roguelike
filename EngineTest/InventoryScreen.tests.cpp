#include "pch.h"
#include "GameWorld.h"
#include "InventoryComponent.h"
#include "InventoryScreen.h"
#include "RenderSystem.h"
#include "UiManager.h"

using RoguelikeGame::InventoryComponent;
using RoguelikeGame::InventoryScreen;
using RoguelikeGame::ItemDefinition;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::RenderSystem;
using XYZEngine::UiManager;

namespace
{
	ItemDefinition MakeItem(const std::string& id, const std::string& name, bool stackable = false)
	{
		ItemDefinition item;
		item.id = id;
		item.name = name;
		item.stackable = stackable;
		item.maxStack = stackable ? 5 : 1;
		return item;
	}

	class InventoryScreenTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			UiManager::Instance()->Clear();
			RenderSystem::Instance()->HandleResize(1280, 720);
		}

		void TearDown() override
		{
			UiManager::Instance()->Clear();
			GameWorld::Instance()->Clear();
		}

		InventoryComponent* CreateInventory()
		{
			GameObject* owner = GameWorld::Instance()->CreateGameObject("Player");
			auto inventory = owner->AddComponent<InventoryComponent>();
			inventory->SetCapacity(12);

			return inventory;
		}
	};
}

TEST_F(InventoryScreenTest, ScreenStartsClosedAndHidden)
{
	InventoryScreen screen;

	EXPECT_FALSE(screen.IsOpen());
	EXPECT_FALSE(screen.IsVisible());
}

TEST_F(InventoryScreenTest, OpenShowsScreenAndCloseHidesItAfterAnimation)
{
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.Open();
	EXPECT_TRUE(screen.IsOpen());
	EXPECT_TRUE(screen.IsVisible());

	screen.Close();
	EXPECT_FALSE(screen.IsOpen());

	for (int frame = 0; frame < 30; frame++)
	{
		screen.Update(0.016f);
	}

	EXPECT_FALSE(screen.IsVisible());
}

TEST_F(InventoryScreenTest, SlotsShowItemNameAndCount)
{
	InventoryComponent* inventory = CreateInventory();
	ItemDefinition potion = MakeItem("potion", "Aptechka", true);
	ItemDefinition key = MakeItem("key", "Klyuch");
	inventory->TryAdd(potion, 3);
	inventory->TryAdd(key);

	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);

	EXPECT_EQ(screen.GetSlotName(0).GetText(), sf::String("Aptechka"));
	EXPECT_EQ(screen.GetSlotCount(0).GetText(), sf::String("3"));
	EXPECT_TRUE(screen.GetSlotCount(0).IsVisible());

	EXPECT_EQ(screen.GetSlotName(1).GetText(), sf::String("Klyuch"));
	EXPECT_FALSE(screen.GetSlotCount(1).IsVisible());

	EXPECT_EQ(screen.GetSlotName(2).GetText(), sf::String(""));
}

TEST_F(InventoryScreenTest, SelectedSlotIsHighlighted)
{
	InventoryComponent* inventory = CreateInventory();
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);

	sf::Color unselected = screen.GetSlotPanel(1).GetShape().getFillColor();

	screen.Open();
	sf::Vector2f point = {screen.GetSlotPanel(1).GetBounds().left + 5.f, screen.GetSlotPanel(1).GetBounds().top + 5.f};
	screen.HandlePointer(point, false, true);

	EXPECT_EQ(screen.GetSelectedSlot(), 1);
	EXPECT_NE(screen.GetSlotPanel(1).GetShape().getFillColor(), unselected);
	EXPECT_EQ(inventory->GetSelectedSlot(), 1);
}

TEST_F(InventoryScreenTest, ScreenFollowsInventoryChanges)
{
	InventoryComponent* inventory = CreateInventory();
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);

	ItemDefinition potion = MakeItem("potion", "Aptechka", true);
	inventory->TryAdd(potion, 2);

	EXPECT_EQ(screen.GetSlotName(0).GetText(), sf::String("Aptechka"));
	EXPECT_EQ(screen.GetSlotCount(0).GetText(), sf::String("2"));

	inventory->Remove(0, 2);

	EXPECT_EQ(screen.GetSlotName(0).GetText(), sf::String(""));
	EXPECT_FALSE(screen.GetSlotCount(0).IsVisible());
}

TEST_F(InventoryScreenTest, ClosedScreenIgnoresPointer)
{
	InventoryComponent* inventory = CreateInventory();
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);

	EXPECT_FALSE(screen.HandlePointer({640.f, 360.f}, false, true));
	EXPECT_EQ(screen.GetSelectedSlot(), 0);
}
