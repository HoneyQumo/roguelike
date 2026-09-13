#include "pch.h"
#include "GameWorld.h"
#include "BoxColliderComponent.h"
#include "RigidbodyComponent.h"
#include "FactionComponent.h"
#include "LevelExitComponent.h"

using RoguelikeGame::Faction;
using RoguelikeGame::FactionComponent;
using RoguelikeGame::LevelExitComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	constexpr float TILE = 64.f;

	class LevelExitGateTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			exitObject = GameWorld::Instance()->CreateGameObject("LevelExit");
			exitObject->GetTransform()->SetWorldPosition({0.f, 0.f});
			auto exitCollider = exitObject->AddComponent<XYZEngine::BoxColliderComponent>();
			exitCollider->SetSize(TILE, TILE);
			exitCollider->SetTrigger(true);
			levelExit = exitObject->AddComponent<LevelExitComponent>();

			entered = 0;
			blocked = 0;
			levelExit->SubscribeEntered([this]() { entered++; });
			levelExit->SubscribeBlocked([this]() { blocked++; });
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* exitObject = nullptr;
		LevelExitComponent* levelExit = nullptr;
		GameObject* player = nullptr;
		int entered = 0;
		int blocked = 0;

		GameObject* CreateWalker(const std::string& name, Faction faction, float x, float y)
		{
			GameObject* walker = GameWorld::Instance()->CreateGameObject(name);
			walker->GetTransform()->SetWorldPosition({x, y});
			walker->AddComponent<FactionComponent>()->SetFaction(faction);
			walker->AddComponent<XYZEngine::RigidbodyComponent>()->SetKinematic(false);

			auto collider = walker->AddComponent<XYZEngine::BoxColliderComponent>();
			collider->SetSize(30.f, 30.f);

			return walker;
		}

		void CreatePlayerAway()
		{
			player = CreateWalker("Player", Faction::Player, 10.f * TILE, 0.f);
			Step();
		}

		void MovePlayerTo(float x, float y)
		{
			player->GetTransform()->SetWorldPosition({x, y});
			Step();
		}

		void Step()
		{
			GameWorld::Instance()->Update(0.016f);
			GameWorld::Instance()->UpdatePhysics();
		}
	};
}

TEST_F(LevelExitGateTest, OpenExitLetsThePlayerThrough)
{
	CreatePlayerAway();

	MovePlayerTo(0.f, 0.f);

	EXPECT_EQ(entered, 1);
	EXPECT_EQ(blocked, 0);
	EXPECT_TRUE(levelExit->IsUsed());
}

TEST_F(LevelExitGateTest, LockedExitBlocksThePlayer)
{
	levelExit->SetLocked(true);
	CreatePlayerAway();

	MovePlayerTo(0.f, 0.f);

	EXPECT_EQ(entered, 0);
	EXPECT_EQ(blocked, 1);
	EXPECT_TRUE(levelExit->IsLocked());
}

TEST_F(LevelExitGateTest, BlockedExitIsNotSpentAndWorksAfterUnlock)
{
	levelExit->SetLocked(true);
	CreatePlayerAway();
	MovePlayerTo(0.f, 0.f);
	ASSERT_FALSE(levelExit->IsUsed());

	MovePlayerTo(10.f * TILE, 0.f);
	levelExit->SetLocked(false);
	MovePlayerTo(0.f, 0.f);

	EXPECT_EQ(entered, 1);
	EXPECT_TRUE(levelExit->IsUsed());
}

TEST_F(LevelExitGateTest, UnlockCatchesUpWithThePlayerStandingOnIt)
{
	levelExit->SetLocked(true);
	CreatePlayerAway();
	MovePlayerTo(0.f, 0.f);
	ASSERT_EQ(entered, 0);

	levelExit->SetLocked(false);

	EXPECT_EQ(entered, 1);
	EXPECT_TRUE(levelExit->IsUsed());
}

TEST_F(LevelExitGateTest, UnlockDoesNothingWhenThePlayerLeft)
{
	levelExit->SetLocked(true);
	CreatePlayerAway();
	MovePlayerTo(0.f, 0.f);
	MovePlayerTo(10.f * TILE, 0.f);

	levelExit->SetLocked(false);

	EXPECT_EQ(entered, 0);
	EXPECT_FALSE(levelExit->IsUsed());
}

TEST_F(LevelExitGateTest, EnemiesDoNotOpenTheExit)
{
	CreateWalker("Enemy", Faction::Enemy, 10.f * TILE, 0.f);
	Step();

	GameWorld::Instance()->FindGameObject("Enemy")->GetTransform()->SetWorldPosition({0.f, 0.f});
	Step();

	EXPECT_EQ(entered, 0);
	EXPECT_EQ(blocked, 0);
}

TEST_F(LevelExitGateTest, ExitFiresOnlyOnce)
{
	CreatePlayerAway();

	MovePlayerTo(0.f, 0.f);
	MovePlayerTo(10.f * TILE, 0.f);
	MovePlayerTo(0.f, 0.f);

	EXPECT_EQ(entered, 1);
}

TEST_F(LevelExitGateTest, LockedExitBlocksOnEveryApproach)
{
	levelExit->SetLocked(true);
	CreatePlayerAway();

	MovePlayerTo(0.f, 0.f);
	MovePlayerTo(10.f * TILE, 0.f);
	MovePlayerTo(0.f, 0.f);

	EXPECT_EQ(blocked, 2);
	EXPECT_EQ(entered, 0);
}
