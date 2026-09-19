#include "pch.h"
#include "DoorComponent.h"
#include "FactionComponent.h"
#include "Openable.h"
#include "PlateComponent.h"
#include "TreadComponent.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <RigidbodyComponent.h>

using RoguelikeGame::DoorComponent;
using RoguelikeGame::Faction;
using RoguelikeGame::FactionComponent;
using RoguelikeGame::LinkSwitches;
using RoguelikeGame::OpenableOf;
using RoguelikeGame::PlateComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	constexpr float TILE = 64.f;
	constexpr float AWAY = 10.f * TILE;

	class PlateTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			GameObject* object = GameWorld::Instance()->CreateGameObject("Plate");
			object->GetTransform()->SetWorldPosition({0.f, 0.f});

			auto pad = object->AddComponent<XYZEngine::BoxColliderComponent>();
			pad->SetSize(TILE, TILE);
			pad->SetTrigger(true);

			auto tread = object->AddComponent<RoguelikeGame::TreadComponent>();
			tread->SetPad(pad);

			plate = object->AddComponent<PlateComponent>();
			plate->SetSwitchId("vault");
			plate->SetTread(tread);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		DoorComponent* CreateDoor(const std::string& id)
		{
			GameObject* object = GameWorld::Instance()->CreateGameObject("Door");
			object->GetTransform()->SetWorldPosition({AWAY, AWAY});
			object->AddComponent<XYZEngine::BoxColliderComponent>()->SetSize(TILE, TILE);

			auto door = object->AddComponent<DoorComponent>();
			door->SetDoorId(id);
			LinkSwitches({plate}, {OpenableOf(door)});

			return door;
		}

		GameObject* CreateWalker(Faction faction)
		{
			GameObject* walker = GameWorld::Instance()->CreateGameObject("Walker");
			walker->GetTransform()->SetWorldPosition({AWAY, 0.f});
			walker->AddComponent<FactionComponent>()->SetFaction(faction);
			walker->AddComponent<XYZEngine::RigidbodyComponent>()->SetKinematic(false);

			auto collider = walker->AddComponent<XYZEngine::BoxColliderComponent>();
			collider->SetSize(30.f, 30.f);

			Step();

			return walker;
		}

		void MoveTo(GameObject* walker, float x)
		{
			walker->GetTransform()->SetWorldPosition({x, 0.f});
			Step();
		}

		void Step()
		{
			GameWorld::Instance()->Update(0.016f);
			GameWorld::Instance()->UpdatePhysics();
		}

		PlateComponent* plate = nullptr;
	};
}

TEST_F(PlateTest, SteppingOnThePlateOpensTheDoorWithTheSameName)
{
	DoorComponent* door = CreateDoor("vault");
	GameObject* player = CreateWalker(Faction::Player);

	MoveTo(player, 0.f);

	EXPECT_TRUE(plate->IsPulled());
	EXPECT_TRUE(door->IsOpen());
}

// Плитка ждёт беглеца, а не охрану: иначе первый же патруль вскроет тайник.
TEST_F(PlateTest, AGuardWalksOverThePlateForNothing)
{
	DoorComponent* door = CreateDoor("vault");
	GameObject* guard = CreateWalker(Faction::Enemy);

	MoveTo(guard, 0.f);

	EXPECT_FALSE(plate->IsPulled());
	EXPECT_FALSE(door->IsOpen());
}

TEST_F(PlateTest, ThePlateSinksOnceAndStaysDown)
{
	int pulls = 0;
	plate->SubscribePulled([&pulls]() { pulls++; });
	GameObject* player = CreateWalker(Faction::Player);

	MoveTo(player, 0.f);
	MoveTo(player, AWAY);
	MoveTo(player, 0.f);

	EXPECT_EQ(pulls, 1);
}

// Нажать плитку нельзя - на неё наступают, поэтому подсказки у неё нет.
TEST_F(PlateTest, ThePlateOffersNothingToPress)
{
	EXPECT_TRUE(plate->GetPrompt(nullptr).empty());
	EXPECT_FALSE(plate->IsAvailable());
	EXPECT_FALSE(plate->Interact(nullptr));
}
