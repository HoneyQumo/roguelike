#include "pch.h"
#include "ChaseComponent.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "PathService.h"
#include "LookAhead.h"
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
	constexpr float NOTICES_AT_ONCE = 100.f;
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
			chase->SetAwareness(NOTICES_AT_ONCE, NOTICES_AT_ONCE);

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

	Vector2Df standing = patrol->GetGameObject()->GetTransform()->GetWorldPosition();
	std::size_t nearest = 0u;
	float best = -1.f;
	for (std::size_t index = 0u; index < patrol->GetPoints().size(); index++)
	{
		float distance = (patrol->GetPoints()[index].position - standing).GetLength();
		if (best < 0.f || distance < best)
		{
			best = distance;
			nearest = index;
		}
	}

	std::size_t after = RoguelikeGame::NextPatrolIndex(nearest, patrol->GetPoints().size());
	EXPECT_TRUE(patrol->GetPointIndex() == nearest || patrol->GetPointIndex() == after)
		<< "index " << patrol->GetPointIndex() << ", nearest " << nearest;
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


// \u0427\u0438\u0441\u0442\u043e\u0435 \u043f\u0440\u0430\u0432\u0438\u043b\u043e \u0431\u0435\u0437 \u0434\u0432\u0438\u0436\u043a\u0430: \u0438\u043c \u043f\u043e\u043b\u044c\u0437\u0443\u044e\u0442\u0441\u044f \u0438 \u043f\u0430\u0442\u0440\u0443\u043b\u044c, \u0438 \u0440\u0430\u0441\u0441\u043b\u0435\u0434\u043e\u0432\u0430\u043d\u0438\u0435 \u0448\u0443\u043c\u0430.
TEST(LookAheadTest, TheLookFollowsTheStep)
{
	Vector2Df position = {100.f, 100.f};
	Vector2Df step = {0.f, 1.f};
	Vector2Df goal = {500.f, 100.f};

	Vector2Df look = RoguelikeGame::LookAheadPoint(position, step, goal, 200.f);

	EXPECT_NEAR(look.x, 100.f, 0.01f);
	EXPECT_NEAR(look.y, 300.f, 0.01f);
}

// \u0428\u0430\u0433\u0430 \u043d\u0435\u0442 - \u043d\u0430\u043f\u0440\u0430\u0432\u043b\u0435\u043d\u0438\u0435 \u0431\u0440\u0430\u0442\u044c \u043d\u0435\u043e\u0442\u043a\u0443\u0434\u0430, \u0441\u0442\u043e\u044f\u0449\u0438\u0439 \u0441\u043c\u043e\u0442\u0440\u0438\u0442 \u043d\u0430 \u0446\u0435\u043b\u044c.
TEST(LookAheadTest, StandingStillLooksAtTheGoal)
{
	Vector2Df goal = {500.f, 100.f};

	Vector2Df look = RoguelikeGame::LookAheadPoint({100.f, 100.f}, {0.f, 0.f}, goal, 200.f);

	EXPECT_NEAR(look.x, goal.x, 0.01f);
	EXPECT_NEAR(look.y, goal.y, 0.01f);
}

TEST(LookAheadTest, TheLookDistanceDoesNotDependOnTheStepLength)
{
	Vector2Df position = {0.f, 0.f};
	Vector2Df goal = {1000.f, 0.f};

	Vector2Df slow = RoguelikeGame::LookAheadPoint(position, {3.f, 0.f}, goal, 150.f);
	Vector2Df fast = RoguelikeGame::LookAheadPoint(position, {300.f, 0.f}, goal, 150.f);

	EXPECT_NEAR(slow.x, fast.x, 0.01f);
	EXPECT_NEAR(slow.y, fast.y, 0.01f);
}

namespace
{
	// \u041f\u0440\u044f\u043c\u043e\u0439 \u043f\u0443\u0442\u044c \u0441\u043d\u0438\u0437\u0443 \u0432\u0432\u0435\u0440\u0445 \u0437\u0430\u043a\u0440\u044b\u0442 \u0441\u0442\u0435\u043d\u043e\u0439: \u0435\u0434\u0438\u043d\u0441\u0442\u0432\u0435\u043d\u043d\u044b\u0439 \u043f\u0440\u043e\u0445\u043e\u0434 - \u0441\u043f\u0440\u0430\u0432\u0430.
	const std::string DETOUR =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"[map]\n"
		"###########\n"
		"#....@....#\n"
		"#########.#\n"
		"#.........#\n"
		"###########\n";
}

// \u0420\u0430\u043d\u044c\u0448\u0435 \u043e\u0445\u0440\u0430\u043d\u043d\u0438\u043a \u0446\u0435\u043b\u0438\u043b\u0441\u044f \u0432 \u0442\u043e\u0447\u043a\u0443 \u043d\u0430\u0437\u043d\u0430\u0447\u0435\u043d\u0438\u044f \u0438 \u043d\u0430 \u043a\u0440\u044e\u043a\u0435 \u0441\u043c\u043e\u0442\u0440\u0435\u043b \u0441\u043a\u0432\u043e\u0437\u044c \u0441\u0442\u0435\u043d\u0443.
// \u041a\u043e\u043d\u0443\u0441 \u0437\u0440\u0435\u043d\u0438\u044f \u0435\u0434\u0435\u0442 \u0437\u0430 \u043f\u0440\u0438\u0446\u0435\u043b\u043e\u043c, \u043f\u043e\u044d\u0442\u043e\u043c\u0443 \u0441\u043f\u0438\u043d\u0430 \u0443 \u043d\u0435\u0433\u043e \u0431\u044b\u043b\u0430 \u043d\u0435 \u0442\u0430\u043c, \u0433\u0434\u0435 \u043a\u0430\u0436\u0435\u0442\u0441\u044f.
TEST_F(PatrolComponentTest, TheGuardLooksWhereItStepsAndNotAtTheGoal)
{
	std::istringstream input(DETOUR);
	LevelGrid::SetCurrent(LevelGrid::Build(LevelLoader::Parse(input, "detour")));
	PathService::Reset();

	Vector2Df goal = At(1, 3);
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), false}, {goal, false}});
	GameObject* guard = patrol->GetGameObject();

	Run(0.4f);

	Vector2Df before = guard->GetTransform()->GetWorldPosition();
	Run(0.2f);
	Vector2Df after = guard->GetTransform()->GetWorldPosition();

	Vector2Df moved = after - before;
	ASSERT_GT(moved.GetLength(), 10.f) << "\u043e\u0445\u0440\u0430\u043d\u043d\u0438\u043a \u043d\u0435 \u0438\u0434\u0451\u0442";

	Vector2Df toGoal = (goal - after).Normalized();
	Vector2Df heading = moved.Normalized();

	// \u0415\u0441\u043b\u0438 \u043a\u0430\u0440\u0442\u0430 \u043f\u0435\u0440\u0435\u0441\u0442\u0430\u043d\u0435\u0442 \u0434\u0430\u0432\u0430\u0442\u044c \u043a\u0440\u044e\u043a, \u043f\u0440\u043e\u0432\u0435\u0440\u044f\u0442\u044c \u0431\u0443\u0434\u0435\u0442 \u043d\u0435\u0447\u0435\u0433\u043e - \u043b\u043e\u0432\u0438\u043c \u044d\u0442\u043e \u0441\u0440\u0430\u0437\u0443.
	ASSERT_LT(heading.DotProduct(toGoal), 0.7f) << "\u043c\u0430\u0440\u0448\u0440\u0443\u0442 \u043d\u0435 \u0433\u043d\u0451\u0442\u0441\u044f, \u0442\u0435\u0441\u0442 \u043d\u0438\u0447\u0435\u0433\u043e \u043d\u0435 \u043f\u0440\u043e\u0432\u0435\u0440\u044f\u0435\u0442";

	Vector2Df forward = guard->GetTransform()->GetForward();

	EXPECT_GT(forward.DotProduct(heading), 0.9f) << "\u0432\u0437\u0433\u043b\u044f\u0434 \u043d\u0435 \u043f\u043e \u0445\u043e\u0434\u0443 \u0434\u0432\u0438\u0436\u0435\u043d\u0438\u044f";
	EXPECT_LT(forward.DotProduct(toGoal), 0.7f) << "\u0441\u043c\u043e\u0442\u0440\u0438\u0442 \u0432 \u0442\u043e\u0447\u043a\u0443 \u043d\u0430\u0437\u043d\u0430\u0447\u0435\u043d\u0438\u044f \u0441\u043a\u0432\u043e\u0437\u044c \u0441\u0442\u0435\u043d\u0443";
}

// \u041d\u0430 \u043f\u0440\u044f\u043c\u043e\u0439 \u0434\u043e\u0440\u043e\u0433\u0435 \u043f\u043e\u0432\u0435\u0434\u0435\u043d\u0438\u0435 \u043d\u0435 \u043c\u0435\u043d\u044f\u0435\u0442\u0441\u044f: \u0448\u0430\u0433 \u0438 \u0446\u0435\u043b\u044c \u0441\u043e\u0432\u043f\u0430\u0434\u0430\u044e\u0442.
TEST_F(PatrolComponentTest, OnAStraightRoadTheGuardStillFacesTheGoal)
{
	Vector2Df goal = At(6, 1);
	PatrolComponent* patrol = CreateGuard(1, 1, {{At(1, 1), false}, {goal, false}});
	GameObject* guard = patrol->GetGameObject();

	Run(0.5f);

	Vector2Df toGoal = (goal - guard->GetTransform()->GetWorldPosition()).Normalized();

	EXPECT_GT(guard->GetTransform()->GetForward().DotProduct(toGoal), 0.9f);
}
