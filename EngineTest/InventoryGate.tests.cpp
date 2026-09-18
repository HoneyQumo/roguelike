#include "pch.h"
#include "InventoryAccess.h"
#include "InventoryComponent.h"
#include "InventoryScreen.h"
#include <Engine.h>
#include <GameWorld.h>
#include <RenderSystem.h>
#include <UiManager.h>

using RoguelikeGame::CanUseInventory;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::InventoryScreen;
using XYZEngine::Engine;
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


namespace
{
	// Пауза глобальная: её надо снимать за собой, иначе упадут соседние тесты.
	class PausedInventoryTest : public InventoryGateTest
	{
	protected:
		void TearDown() override
		{
			Engine::Instance()->SetPaused(false);
			InventoryGateTest::TearDown();
		}
	};
}

// На паузе аптечка лечила, патроны списывались, ствол экипировался в замерший мир.
TEST_F(PausedInventoryTest, APausedGameClosesTheBagForBusiness)
{
	InventoryComponent* inventory = CreateInventory();

	ASSERT_TRUE(CanUseInventory(inventory));

	Engine::Instance()->SetPaused(true);

	EXPECT_FALSE(CanUseInventory(inventory)) << "сумка работает на остановленной игре";
}

TEST_F(PausedInventoryTest, TheBagComesBackWhenTheGameDoes)
{
	InventoryComponent* inventory = CreateInventory();

	Engine::Instance()->SetPaused(true);
	Engine::Instance()->SetPaused(false);

	EXPECT_TRUE(CanUseInventory(inventory)) << "сумка не ожила вместе с игрой";
}

// Главный путь: Esc при закрытой сумке ставит паузу, а потом игрок жмёт Tab.
TEST_F(PausedInventoryTest, AnOpenBagShutsWhenTheGameStops)
{
	InventoryComponent* inventory = CreateInventory();

	InventoryScreen screen;
	screen.SetInventory(inventory);
	screen.Open();
	screen.Update(0.016f);
	ASSERT_TRUE(screen.IsOpen());

	Engine::Instance()->SetPaused(true);
	screen.Update(0.016f);

	EXPECT_FALSE(screen.IsOpen()) << "сумка осталась открытой на паузе";
}
