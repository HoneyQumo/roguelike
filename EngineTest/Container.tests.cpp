#include "pch.h"
#include "GameWorld.h"
#include "BoxColliderComponent.h"
#include "ContainerComponent.h"
#include "FactionComponent.h"
#include "GameSettings.h"
#include "InteractionComponent.h"
#include "InventoryComponent.h"
#include "RectangleRendererComponent.h"
#include "RigidbodyComponent.h"

using RoguelikeGame::ContainerComponent;
using RoguelikeGame::Faction;
using RoguelikeGame::FactionComponent;
using RoguelikeGame::InteractionComponent;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	const ItemDefinition& RustyKey()
	{
		static const ItemDefinition key = []()
		{
			ItemDefinition item;
			item.id = "key_rusty";
			item.name = u8"Ржавый ключ";
			item.stackable = false;
			item.maxStack = 1;
			return item;
		}();

		return key;
	}

	class ContainerTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			player = GameWorld::Instance()->CreateGameObject("Player");
			player->AddComponent<FactionComponent>()->SetFaction(Faction::Player);
			inventory = player->AddComponent<InventoryComponent>();
			inventory->SetCapacity(4);
			interaction = player->AddComponent<InteractionComponent>();

			chest = GameWorld::Instance()->CreateGameObject("Prop_chest");
			chest->GetTransform()->SetWorldPosition({100.f, 50.f});
			renderer = chest->AddComponent<XYZEngine::RectangleRendererComponent>();
			renderer->SetSize(48.f, 48.f);
			renderer->SetColor({120, 95, 50});
			container = chest->AddComponent<ContainerComponent>();
			container->SetTitle(u8"Ящик");

			GameWorld::Instance()->Update(0.016f);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		void Lock() { container->SetKeyItem("key_rusty", u8"Ржавый ключ"); }

		GameObject* player = nullptr;
		GameObject* chest = nullptr;
		InventoryComponent* inventory = nullptr;
		InteractionComponent* interaction = nullptr;
		ContainerComponent* container = nullptr;
		XYZEngine::RectangleRendererComponent* renderer = nullptr;
	};
}

TEST_F(ContainerTest, UnlockedContainerOpensRightAway)
{
	Vector2Df place;
	int opened = 0;
	container->SubscribeOpened([&](const Vector2Df& at) { opened++; place = at; });

	EXPECT_FALSE(container->IsLocked());
	EXPECT_TRUE(container->Interact(player));
	EXPECT_TRUE(container->IsOpen());
	EXPECT_EQ(opened, 1);
	EXPECT_FLOAT_EQ(place.x, 100.f);
	EXPECT_FLOAT_EQ(place.y, 50.f);
}

TEST_F(ContainerTest, LockedContainerRefusesWithoutTheKey)
{
	Lock();

	int opened = 0;
	container->SubscribeOpened([&](const Vector2Df&) { opened++; });

	EXPECT_FALSE(container->Interact(player));
	EXPECT_FALSE(container->IsOpen());
	EXPECT_EQ(opened, 0);
}

TEST_F(ContainerTest, RefusalNamesTheMissingKey)
{
	Lock();

	std::string refusal = container->GetRefusal(player);

	EXPECT_NE(refusal.find(u8"Ржавый ключ"), std::string::npos);
	EXPECT_EQ(refusal.find(RoguelikeGame::CONTAINER_LOCKED_PREFIX), 0u);
}

TEST_F(ContainerTest, KeyIsSpentOnOpening)
{
	Lock();
	ASSERT_TRUE(inventory->TryAdd(RustyKey(), 1));

	EXPECT_TRUE(container->Interact(player));
	EXPECT_TRUE(container->IsOpen());
	EXPECT_EQ(inventory->CountOf("key_rusty"), 0);
}

TEST_F(ContainerTest, OpenedContainerGivesNothingMore)
{
	Lock();
	ASSERT_TRUE(inventory->TryAdd(RustyKey(), 1));
	ASSERT_TRUE(inventory->TryAdd(RustyKey(), 1));
	ASSERT_EQ(inventory->CountOf("key_rusty"), 2);
	ASSERT_TRUE(container->Interact(player));

	int opened = 0;
	container->SubscribeOpened([&](const Vector2Df&) { opened++; });

	EXPECT_FALSE(container->Interact(player));
	EXPECT_EQ(opened, 0);
	EXPECT_EQ(inventory->CountOf("key_rusty"), 1);
}

TEST_F(ContainerTest, OpenedContainerLeavesTheCandidates)
{
	interaction->AddCandidate(container);
	GameWorld::Instance()->Update(0.016f);
	ASSERT_EQ(interaction->GetTarget(), container);

	ASSERT_TRUE(container->Interact(player));
	GameWorld::Instance()->Update(0.016f);

	EXPECT_FALSE(container->IsAvailable());
	EXPECT_EQ(interaction->GetTarget(), nullptr);
	EXPECT_TRUE(interaction->GetPrompt().empty());
}

TEST_F(ContainerTest, PromptFollowsTheKeyInTheBag)
{
	Lock();
	interaction->AddCandidate(container);

	GameWorld::Instance()->Update(0.016f);
	EXPECT_NE(interaction->GetPrompt().find(u8"Ржавый ключ"), std::string::npos);

	ASSERT_TRUE(inventory->TryAdd(RustyKey(), 1));
	GameWorld::Instance()->Update(0.016f);

	EXPECT_NE(interaction->GetPrompt().find(u8"Ящик"), std::string::npos);
	EXPECT_EQ(interaction->GetPrompt().find(RoguelikeGame::CONTAINER_OPEN_PREFIX), 0u);
}

TEST_F(ContainerTest, RefusalReachesTheInteractionSubscriber)
{
	Lock();
	interaction->AddCandidate(container);
	GameWorld::Instance()->Update(0.016f);

	std::string reported;
	int refusals = 0;
	interaction->SubscribeRefused([&](const std::string& reason) { refusals++; reported = reason; });

	EXPECT_FALSE(interaction->Interact());
	EXPECT_EQ(refusals, 1);
	EXPECT_NE(reported.find(u8"Ржавый ключ"), std::string::npos);
}

TEST_F(ContainerTest, SuccessReportsNoRefusal)
{
	interaction->AddCandidate(container);
	GameWorld::Instance()->Update(0.016f);

	int refusals = 0;
	interaction->SubscribeRefused([&](const std::string&) { refusals++; });

	EXPECT_TRUE(interaction->Interact());
	EXPECT_EQ(refusals, 0);
}

TEST_F(ContainerTest, PlayerNearTheChestBecomesItsCandidate)
{
	auto reach = chest->AddComponent<XYZEngine::BoxColliderComponent>();
	reach->SetSize(72.f, 72.f);
	reach->SetTrigger(true);
	container->SetReach(reach);

	auto body = player->AddComponent<XYZEngine::RigidbodyComponent>();
	body->SetKinematic(false);
	auto playerCollider = player->AddComponent<XYZEngine::BoxColliderComponent>();
	playerCollider->SetSize(32.f, 32.f);
	player->GetTransform()->SetWorldPosition({100.f, 50.f});

	GameWorld::Instance()->Update(0.016f);
	GameWorld::Instance()->UpdatePhysics();
	GameWorld::Instance()->Update(0.016f);

	EXPECT_EQ(interaction->GetTarget(), container);
}
