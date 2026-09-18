#include "pch.h"
#include "InventoryAccess.h"
#include "InventoryComponent.h"
#include "InventoryScreen.h"
#include <GameWorld.h>
#include <RenderSystem.h>
#include <UiManager.h>

using RoguelikeGame::CanUseInventory;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::InventoryScreen;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::UiManager;

namespace
{
	class InventoryGateTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			UiManager::Instance()->Clear();
			XYZEngine::RenderSystem::Instance()->HandleResize(1280, 720);
			BeginFrame();
		}

		void TearDown() override
		{
			UiManager::Instance()->Clear();
			GameWorld::Instance()->Clear();
		}

		// Движок начинает кадр именно так, и здесь же гаснет захват ввода.
		void BeginFrame()
		{
			UiManager::Instance()->HandleInput();
		}

		InventoryComponent* CreateInventory()
		{
			owner = GameWorld::Instance()->CreateGameObject("Player");
			auto inventory = owner->AddComponent<InventoryComponent>();
			inventory->SetCapacity(12);

			return inventory;
		}

		GameObject* owner = nullptr;
	};
}

TEST_F(InventoryGateTest, AnOpenBagTakesTheGameInput)
{
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(CreateInventory());

	ASSERT_FALSE(UiManager::Instance()->IsInputCaptured());

	screen.Open();
	screen.Update(0.016f);

	EXPECT_TRUE(UiManager::Instance()->IsInputCaptured()) << "мир прочитает те же цифры, что и сумка";
}

TEST_F(InventoryGateTest, TheBagHoldsTheInputEveryFrameItIsOpen)
{
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(CreateInventory());

	screen.Open();

	for (int frame = 0; frame < 5; frame++)
	{
		BeginFrame();
		screen.Update(0.016f);

		ASSERT_TRUE(UiManager::Instance()->IsInputCaptured()) << "захват держится только в кадр открытия";
	}
}

TEST_F(InventoryGateTest, AClosedBagLeavesTheInputToTheWorld)
{
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(CreateInventory());

	screen.Update(0.016f);

	EXPECT_FALSE(UiManager::Instance()->IsInputCaptured());
}

TEST_F(InventoryGateTest, TheBagClosesWhenItsOwnerIsFrozen)
{
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	InventoryComponent* inventory = CreateInventory();
	screen.SetInventory(inventory);

	screen.Open();
	screen.Update(0.016f);
	ASSERT_TRUE(screen.IsOpen());

	inventory->SetEnabled(false);
	BeginFrame();
	screen.Update(0.016f);

	EXPECT_FALSE(screen.IsOpen()) << "в катсцене сумка остаётся открытой";
	EXPECT_FALSE(UiManager::Instance()->IsInputCaptured());
}

TEST_F(InventoryGateTest, ABagIsReachableOnlyWhileItsOwnerIsInTheGame)
{
	InventoryComponent* inventory = CreateInventory();

	EXPECT_TRUE(CanUseInventory(inventory));

	inventory->SetEnabled(false);
	EXPECT_FALSE(CanUseInventory(inventory)) << "замороженная сумка отвечает на вызовы";

	inventory->SetEnabled(true);
	owner->SetActive(false);
	EXPECT_FALSE(CanUseInventory(inventory)) << "выключенный хозяин сумку не останавливает";

	EXPECT_FALSE(CanUseInventory(nullptr));
}
