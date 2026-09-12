#include "pch.h"
#include "GameWorld.h"
#include "BoxColliderComponent.h"
#include "FactionComponent.h"
#include "InteractionComponent.h"
#include "InventoryComponent.h"
#include "ItemPickupComponent.h"

using RoguelikeGame::Faction;
using RoguelikeGame::FactionComponent;
using RoguelikeGame::InteractionComponent;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemPickupComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	ItemDefinition MakeItem(const std::string& id, const std::string& name)
	{
		ItemDefinition item;
		item.id = id;
		item.name = name;
		item.stackable = false;
		item.maxStack = 1;
		return item;
	}

	class InteractionTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* player = nullptr;
		InteractionComponent* interaction = nullptr;
		InventoryComponent* inventory = nullptr;

		void CreatePlayer()
		{
			player = GameWorld::Instance()->CreateGameObject("Player");
			player->AddComponent<FactionComponent>()->SetFaction(Faction::Player);
			inventory = player->AddComponent<InventoryComponent>();
			inventory->SetCapacity(4);
			interaction = player->AddComponent<InteractionComponent>();
		}

		ItemPickupComponent* CreateItem(const ItemDefinition& definition, float x, float y)
		{
			GameObject* item = GameWorld::Instance()->CreateGameObject("Item_" + definition.id);
			item->GetTransform()->SetWorldPosition({x, y});

			auto collider = item->AddComponent<XYZEngine::BoxColliderComponent>();
			collider->SetSize(40.f, 40.f);
			collider->SetTrigger(true);

			auto pickup = item->AddComponent<ItemPickupComponent>();
			pickup->SetDefinition(&definition);

			return pickup;
		}
	};
}

TEST_F(InteractionTest, NoCandidatesMeansNoPrompt)
{
	CreatePlayer();

	EXPECT_EQ(interaction->GetTarget(), nullptr);
	EXPECT_TRUE(interaction->GetPrompt().empty());
	EXPECT_FALSE(interaction->Interact());
}

TEST_F(InteractionTest, CandidateBecomesTargetWithPrompt)
{
	CreatePlayer();
	ItemDefinition rifle = MakeItem("weapon_ak", "AK-47");
	ItemPickupComponent* pickup = CreateItem(rifle, 10.f, 0.f);

	interaction->AddCandidate(pickup);
	player->Update(0.016f);

	EXPECT_EQ(interaction->GetTarget(), pickup);
	EXPECT_NE(interaction->GetPrompt().find("AK-47"), std::string::npos);
}

TEST_F(InteractionTest, NearestCandidateWins)
{
	CreatePlayer();
	player->GetTransform()->SetWorldPosition({0.f, 0.f});

	ItemDefinition far = MakeItem("far_item", "Dalniy");
	ItemDefinition near = MakeItem("near_item", "Blizkiy");
	ItemPickupComponent* farPickup = CreateItem(far, 300.f, 0.f);
	ItemPickupComponent* nearPickup = CreateItem(near, 30.f, 0.f);

	interaction->AddCandidate(farPickup);
	interaction->AddCandidate(nearPickup);
	player->Update(0.016f);

	EXPECT_EQ(interaction->GetTarget(), nearPickup);
	EXPECT_NE(interaction->GetPrompt().find("Blizkiy"), std::string::npos);
}

TEST_F(InteractionTest, LeavingTheZoneClearsThePrompt)
{
	CreatePlayer();
	ItemDefinition potion = MakeItem("potion", "Aptechka");
	ItemPickupComponent* pickup = CreateItem(potion, 10.f, 0.f);

	interaction->AddCandidate(pickup);
	player->Update(0.016f);
	ASSERT_FALSE(interaction->GetPrompt().empty());

	interaction->RemoveCandidate(pickup);

	EXPECT_EQ(interaction->GetTarget(), nullptr);
	EXPECT_TRUE(interaction->GetPrompt().empty());
}

TEST_F(InteractionTest, InteractPutsItemIntoInventory)
{
	CreatePlayer();
	ItemDefinition potion = MakeItem("potion", "Aptechka");
	ItemPickupComponent* pickup = CreateItem(potion, 10.f, 0.f);

	interaction->AddCandidate(pickup);
	player->Update(0.016f);

	EXPECT_TRUE(interaction->Interact());

	EXPECT_TRUE(inventory->Contains("potion"));
	EXPECT_TRUE(pickup->IsPickedUp());
	EXPECT_EQ(interaction->GetTarget(), nullptr);
}

TEST_F(InteractionTest, ItemStaysOnTheGroundWhenInventoryIsFull)
{
	CreatePlayer();
	inventory->SetCapacity(1);
	ItemDefinition first = MakeItem("first", "Perviy");
	ItemDefinition second = MakeItem("second", "Vtoroy");

	inventory->TryAdd(first);

	ItemPickupComponent* pickup = CreateItem(second, 10.f, 0.f);
	interaction->AddCandidate(pickup);
	player->Update(0.016f);

	EXPECT_FALSE(interaction->Interact());

	EXPECT_FALSE(pickup->IsPickedUp());
	EXPECT_FALSE(inventory->Contains("second"));
	EXPECT_EQ(interaction->GetTarget(), pickup);
}

TEST_F(InteractionTest, PromptChangesAreReported)
{
	CreatePlayer();
	ItemDefinition potion = MakeItem("potion", "Aptechka");
	ItemPickupComponent* pickup = CreateItem(potion, 10.f, 0.f);

	std::vector<std::string> prompts;
	interaction->SubscribePromptChanged([&prompts](const std::string& prompt) { prompts.push_back(prompt); });

	interaction->AddCandidate(pickup);
	player->Update(0.016f);
	interaction->RemoveCandidate(pickup);

	ASSERT_EQ(prompts.size(), 2u);
	EXPECT_NE(prompts[0].find("Aptechka"), std::string::npos);
	EXPECT_TRUE(prompts[1].empty());
}

TEST_F(InteractionTest, PickedUpCandidateIsForgotten)
{
	CreatePlayer();
	ItemDefinition potion = MakeItem("potion", "Aptechka");
	ItemPickupComponent* pickup = CreateItem(potion, 10.f, 0.f);

	interaction->AddCandidate(pickup);
	player->Update(0.016f);
	pickup->TryPickUp(player);

	player->Update(0.016f);

	EXPECT_EQ(interaction->GetTarget(), nullptr);
	EXPECT_TRUE(interaction->GetPrompt().empty());
}
