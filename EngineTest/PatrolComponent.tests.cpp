#include "pch.h"
#include "ChaseComponent.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "PathService.h"
#include "PatrolComponent.h"
#include "PatrolRoutes.h"
#include <AimRotationComponent.h>
#include <GameWorld.h>
#include <MovementComponent.h>
#include <sstream>

using namespace XYZEngine;
using RoguelikeGame::ChaseComponent;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PatrolComponent;
using RoguelikeGame::PathService;
using RoguelikeGame::PatrolStop;

namespace
{
	const std::string HALL =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"[map]\n"
		"########\n"
		"#......#\n"
		"#..@...#\n"
		"#......#\n"
		"########\n";

	constexpr float STEP = 0.05f;
	constexpr float LOOK_TIME = 2.f;

	class PatrolComponentTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			std::istringstream input(HALL);
			LevelData level = LevelLoader::Parse(input, "patrol");
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

		PatrolComponent* CreateGuard(int column, int row, std::vector<PatrolStop> points)
		{
			GameObject* guard = GameWorld::Instance()->CreateGameObject("Guard");
			guard->GetTransform()->SetWorldPosition(At(column, row));
			guard->GetTransform()->SetWorldRotation(0.f);

			guard->AddComponent<MovementComponent>()->SetSpeed(150.f);
			guard->AddComponent<AimRotationComponent>()->SetMaxDistance(0.f);

			auto chase = guard->AddComponent<ChaseComponent>();
			chase->SetTargetName("Hero");
			chase->SetDetectionRadius(400.f);
			chase->SetStopDistance(40.f);
			chase->SetVisionHalfAngle(45.f);
			chase->SetAlertHalfAngle(60.f);
			chase->SetSearchTime(3.f);

			auto patrol = guard->AddComponent<PatrolComponent>();
			patrol->SetLook(LOOK_TIME, 40.f);
			patrol->SetPoints(std::move(points));

			return patrol;
		}

		bool WaitForIndex(PatrolComponent* patrol, std::size_t wanted)
		{
			for (int attempt = 0; attempt < 400; attempt++)
			{
				Run(STEP);
				if (patrol->GetPointIndex() == wanted)
				{
					return true;
				}
			}

			return false;
		}

		bool WaitForPatrol(PatrolComponent* patrol)
		{
			for (int attempt = 0; attempt < 400; attempt++)
			{
				Run(STEP);
				if (patrol->IsWalking())
				{
					return true;
				}
			}

			return false;
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

TEST_F(PatrolComponentTest, GuardWalksToTheNextPoint)
{
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), false}, {At(6, 1), false}});
	GameObject* guard = patrol->GetGameObject();

	float before = (At(6, 1) - guard->GetTransform()->GetWorldPosition()).GetLength();
	Run(1.f);
	float after = (At(6, 1) - guard->GetTransform()->GetWorldPosition()).GetLength();

	EXPECT_LT(after, before - 50.f);
}

TEST_F(PatrolComponentTest, GuardTurnsTowardsWhereItWalks)
{
	PatrolComponent* patrol = CreateGuard(6, 1, {{At(6, 1), false}, {At(1, 1), false}});
	GameObject* guard = patrol->GetGameObject();

	Run(0.5f);

	Vector2Df facing = DirectionFromDegrees(guard->GetTransform()->GetWorldRotation());
	EXPECT_LT(facing.x, 0.f);
}

TEST_F(PatrolComponentTest, RouteLoopsRoundAndRound)
{
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), false}, {At(6, 1), false}});

	Run(0.2f);
	ASSERT_EQ(patrol->GetPointIndex(), 1u);

	EXPECT_TRUE(WaitForIndex(patrol, 0u));
	EXPECT_TRUE(WaitForIndex(patrol, 1u));
}

TEST_F(PatrolComponentTest, GuardWithOnePointStandsAndLooksAtIt)
{
	PatrolComponent* patrol = CreateGuard(3, 2, {{At(3, 1), false}});
	GameObject* guard = patrol->GetGameObject();
	Vector2Df before = guard->GetTransform()->GetWorldPosition();

	Run(1.f);

	Vector2Df after = guard->GetTransform()->GetWorldPosition();
	EXPECT_FLOAT_EQ(after.x, before.x);
	EXPECT_FLOAT_EQ(after.y, before.y);

	Vector2Df facing = DirectionFromDegrees(guard->GetTransform()->GetWorldRotation());
	EXPECT_GT(facing.y, 0.f);
}

TEST_F(PatrolComponentTest, GuardWithoutARouteDoesNothing)
{
	PatrolComponent* patrol = CreateGuard(3, 2, {});
	GameObject* guard = patrol->GetGameObject();
	Vector2Df before = guard->GetTransform()->GetWorldPosition();

	Run(1.f);

	EXPECT_FLOAT_EQ(guard->GetTransform()->GetWorldPosition().x, before.x);
	EXPECT_FALSE(patrol->IsWalking());
}

TEST_F(PatrolComponentTest, ChaseStopsThePatrol)
{
	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition(At(3, 1));

	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), false}, {At(6, 1), false}});
	Run(0.2f);

	EXPECT_FALSE(patrol->IsWalking());
}

TEST_F(PatrolComponentTest, GuardReturnsToTheNearestPointAfterTheChase)
{
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), false}, {At(6, 1), false}, {At(6, 3), false}});
	Run(0.5f);
	ASSERT_EQ(patrol->GetPointIndex(), 1u);

	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition(At(6, 3));
	Run(0.2f);
	ASSERT_FALSE(patrol->IsWalking());

	Run(3.f);
	GameWorld::Instance()->DestroyGameObject(hero);
	GameWorld::Instance()->LateUpdate();

	ASSERT_TRUE(WaitForPatrol(patrol));
	EXPECT_EQ(patrol->GetPointIndex(), 2u);
}

TEST_F(PatrolComponentTest, GuardStopsAtAWatchPoint)
{
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), true}, {At(6, 1), false}});

	Run(0.2f);

	EXPECT_TRUE(patrol->IsLooking());
	EXPECT_FALSE(patrol->IsWalking());
	EXPECT_EQ(patrol->GetPointIndex(), 0u);
}

TEST_F(PatrolComponentTest, GuardDoesNotStopAtAPlainPoint)
{
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), false}, {At(6, 1), false}});

	Run(0.2f);

	EXPECT_FALSE(patrol->IsLooking());
	EXPECT_EQ(patrol->GetPointIndex(), 1u);
}

TEST_F(PatrolComponentTest, GuardTurnsBothWaysWhileLooking)
{
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), true}, {At(6, 1), false}});
	GameObject* guard = patrol->GetGameObject();
	Run(0.2f);
	ASSERT_TRUE(patrol->IsLooking());

	float least = guard->GetTransform()->GetWorldRotation();
	float most = least;
	for (int step = 0; step < 40; step++)
	{
		Run(STEP);
		float angle = guard->GetTransform()->GetWorldRotation();
		least = std::min(least, angle);
		most = std::max(most, angle);
	}

	EXPECT_GT(most - least, 30.f);
}

TEST_F(PatrolComponentTest, LookEndsAndTheGuardWalksOn)
{
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), true}, {At(6, 1), false}});
	Run(0.2f);
	ASSERT_TRUE(patrol->IsLooking());

	Run(LOOK_TIME + 0.2f);

	EXPECT_FALSE(patrol->IsLooking());
	EXPECT_EQ(patrol->GetPointIndex(), 1u);
	EXPECT_TRUE(patrol->IsWalking());
}

TEST_F(PatrolComponentTest, SeeingTheTargetBreaksTheLook)
{
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), true}, {At(6, 1), false}});
	Run(0.2f);
	ASSERT_TRUE(patrol->IsLooking());

	GameObject* hero = GameWorld::Instance()->CreateGameObject("Hero");
	hero->GetTransform()->SetWorldPosition(At(3, 1));
	Run(0.2f);

	EXPECT_FALSE(patrol->IsLooking());
}

TEST_F(PatrolComponentTest, GuardWithoutLookTimeWalksThroughWatchPoints)
{
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), true}, {At(6, 1), false}});
	patrol->SetLook(0.f, 40.f);

	Run(0.2f);

	EXPECT_FALSE(patrol->IsLooking());
	EXPECT_EQ(patrol->GetPointIndex(), 1u);
}
