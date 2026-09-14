#include "pch.h"
#include "ChaseComponent.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "PathService.h"
#include <AimRotationComponent.h>
#include <GameWorld.h>
#include <MovementComponent.h>
#include "EnemyAttackComponent.h"
#include <sstream>

using namespace XYZEngine;
using RoguelikeGame::ChaseComponent;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PathService;

namespace
{
	const std::string ROOM =
		"[map]\n"
		"#############\n"
		"#...#.......#\n"
		"#...........#\n"
		"#...#.......#\n"
		"#############\n";

	constexpr float SEARCH_TIME = 4.f;
	constexpr float STEP = 0.05f;
	constexpr float LOOK_TIME = 1.f;

	class ChaseComponentTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			std::istringstream input(ROOM);
			LevelData level = LevelLoader::Parse(input, "chase");
			LevelGrid::SetCurrent(LevelGrid::Build(level));
			PathService::Reset();
		}

		void TearDown() override
		{
			GameWorld::Instance()->Clear();
			LevelGrid::SetCurrent(LevelGrid());
			PathService::Reset();
		}

		Vector2Df At(int column, int row) const
		{
			return LevelGrid::Current().ToWorld(column, row);
		}

		GameObject* CreateHero(int column, int row)
		{
			GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
			hero->GetTransform()->SetWorldPosition(At(column, row));

			return hero;
		}

		ChaseComponent* CreateEnemy(int column, int row)
		{
			GameObject* enemy = GameWorld::Instance()->CreateGameObject("Watcher");
			enemy->GetTransform()->SetWorldPosition(At(column, row));
			enemy->GetTransform()->SetWorldRotation(0.f);

			enemy->AddComponent<MovementComponent>()->SetSpeed(120.f);
			enemy->AddComponent<AimRotationComponent>();

			auto chase = enemy->AddComponent<ChaseComponent>();
			chase->SetTargetName("Hero");
			chase->SetDetectionRadius(400.f);
			chase->SetStopDistance(40.f);
			chase->SetVisionHalfAngle(45.f);
			chase->SetAlertHalfAngle(60.f);
			chase->SetAlertTime(5.f);
			chase->SetSearchTime(SEARCH_TIME);
			chase->SetLook(LOOK_TIME, 40.f);

			return chase;
		}

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += STEP)
			{
				GameWorld::Instance()->Update(STEP);
			}
		}
	};
}

TEST_F(ChaseComponentTest, TargetInSightIsChased)
{
	CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);

	Run(0.2f);

	EXPECT_TRUE(chase->IsChasing());
	EXPECT_TRUE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, TargetBehindAWallIsNotNoticed)
{
	CreateHero(5, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);

	Run(1.f);

	EXPECT_FALSE(chase->IsChasing());
	EXPECT_FALSE(chase->IsAlerted());
}

TEST_F(ChaseComponentTest, TargetBehindTheBackIsNotNoticed)
{
	CreateHero(1, 1);
	ChaseComponent* chase = CreateEnemy(3, 1);

	Run(1.f);

	EXPECT_FALSE(chase->IsChasing());
	EXPECT_FALSE(chase->IsAlerted());
}

TEST_F(ChaseComponentTest, LostTargetLeavesTheEnemySearching)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.2f);

	EXPECT_FALSE(chase->IsChasing());
	EXPECT_TRUE(chase->IsAlerted());
	EXPECT_TRUE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, EnemyWalksToThePlaceItLastSawTheTarget)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	Vector2Df lastSeen = hero->GetTransform()->GetWorldPosition();
	hero->GetTransform()->SetWorldPosition(At(5, 1));

	GameObject* enemy = chase->GetGameObject();
	float before = (lastSeen - enemy->GetTransform()->GetWorldPosition()).GetLength();
	ASSERT_GT(before, RoguelikeGame::ENEMY_ALERT_ARRIVE_DISTANCE);

	Run(2.f);
	float after = (lastSeen - enemy->GetTransform()->GetWorldPosition()).GetLength();

	EXPECT_LE(after, RoguelikeGame::ENEMY_ALERT_ARRIVE_DISTANCE);
}

TEST_F(ChaseComponentTest, SearchEndsWhenTheTimeRunsOut)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.2f);
	ASSERT_TRUE(chase->IsAlerted());

	Run(SEARCH_TIME + 1.f);

	EXPECT_FALSE(chase->IsAlerted());
	EXPECT_FALSE(chase->IsEngaged());
}

TEST_F(ChaseComponentTest, EnemyThatNeverSawAnyoneStaysPut)
{
	CreateHero(5, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	GameObject* enemy = chase->GetGameObject();
	Vector2Df before = enemy->GetTransform()->GetWorldPosition();

	Run(2.f);

	Vector2Df after = enemy->GetTransform()->GetWorldPosition();
	EXPECT_FLOAT_EQ(after.x, before.x);
	EXPECT_FLOAT_EQ(after.y, before.y);
}

TEST_F(ChaseComponentTest, SeeingTheTargetAgainStartsANewSearch)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(SEARCH_TIME);
	ASSERT_FALSE(chase->IsAlerted());

	hero->GetTransform()->SetWorldPosition(At(3, 1));
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.2f);

	EXPECT_TRUE(chase->IsAlerted());
}

TEST_F(ChaseComponentTest, EnemyWithoutSearchTimeForgetsAtOnce)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetSearchTime(0.f);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.2f);

	EXPECT_FALSE(chase->IsAlerted());
}

TEST_F(ChaseComponentTest, EmptyLastSeenPlaceEndsTheSearchQuickly)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	GameWorld::Instance()->DestroyGameObject(hero);
	GameWorld::Instance()->LateUpdate();

	Run(0.2f);
	ASSERT_TRUE(chase->IsAlerted());

	Run(SEARCH_TIME - 1.f);

	EXPECT_FALSE(chase->IsAlerted());
}

TEST_F(ChaseComponentTest, EnemyWithSearchSpotsGoesToCheckThem)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* seeker = CreateEnemy(1, 1);
	seeker->SetSearchSpots(10, 2, 2);
	seeker->SetSearchLook(0.3f);
	seeker->SetSearchGap(1);
	Run(0.2f);
	ASSERT_TRUE(seeker->IsChasing());

	GameWorld::Instance()->DestroyGameObject(hero);
	GameWorld::Instance()->LateUpdate();
	Run(20.f);

	EXPECT_GE(seeker->GetSearchStep(), 1u);
}

TEST_F(ChaseComponentTest, AlertedEnemySeesFurther)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetDetectionRadius(100.f);
	chase->SetAlertRadiusScale(2.f);
	chase->GetGameObject()->GetComponent<MovementComponent>()->SetSpeed(0.f);

	Run(0.2f);
	EXPECT_FALSE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(2, 1));
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(3, 1));
	Run(0.2f);

	EXPECT_TRUE(chase->IsChasing());
}

TEST_F(ChaseComponentTest, SeeingTheTargetDuringTheSearchResumesTheChase)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetSearchSpots(10, 2, 2);
	chase->SetSearchLook(2.f);
	Run(0.2f);
	ASSERT_TRUE(chase->IsChasing());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(1.f);
	ASSERT_FALSE(chase->IsChasing());
	ASSERT_TRUE(chase->IsAlerted());

	hero->GetTransform()->SetWorldPosition(At(4, 2));
	Run(0.3f);

	EXPECT_TRUE(chase->IsChasing());
	EXPECT_EQ(chase->GetSearchStep(), 0u);
}

TEST_F(ChaseComponentTest, AlertedEnemyBehindAWallCannotSeeTheTarget)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetSearchSpots(10, 2, 2);
	chase->SetSearchLook(2.f);
	Run(0.2f);
	ASSERT_TRUE(chase->CanSeeTarget());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.5f);

	EXPECT_TRUE(chase->IsAlerted());
	EXPECT_FALSE(chase->CanSeeTarget());
}

TEST_F(ChaseComponentTest, AlertedEnemyDoesNotAttackThroughAWall)
{
	GameObject* hero = CreateHero(3, 1);
	ChaseComponent* chase = CreateEnemy(1, 1);
	chase->SetSearchSpots(10, 2, 2);
	chase->SetSearchLook(2.f);

	GameObject* enemy = chase->GetGameObject();
	auto attack = enemy->AddComponent<RoguelikeGame::EnemyAttackComponent>();
	attack->SetTargetName("Hero");
	attack->SetAttackRange(1000.f);

	Run(0.2f);
	ASSERT_TRUE(chase->CanSeeTarget());
	EXPECT_TRUE(attack->IsAttacking());

	hero->GetTransform()->SetWorldPosition(At(5, 1));
	Run(0.5f);

	ASSERT_TRUE(chase->IsAlerted());
	ASSERT_FALSE(chase->CanSeeTarget());
	EXPECT_FALSE(attack->IsAttacking());
}
