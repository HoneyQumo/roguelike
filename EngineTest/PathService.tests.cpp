#include "pch.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "PathService.h"
#include <sstream>

using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PathField;
using RoguelikeGame::PathService;
using RoguelikeGame::PATH_FIELD_SLOTS;
using XYZEngine::Vector2Df;

namespace
{
	const std::string HALL =
		"[map]\n"
		"##########\n"
		"#........#\n"
		"#........#\n"
		"#####.####\n"
		"#........#\n"
		"##########\n";

	class PathServiceTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			std::istringstream input(HALL);
			LevelData level = LevelLoader::Parse(input, "hall");
			LevelGrid::SetCurrent(LevelGrid::Build(level));
			PathService::Reset();
		}

		void TearDown() override
		{
			LevelGrid::SetCurrent(LevelGrid());
			PathService::Reset();
		}

		Vector2Df At(int column, int row) const
		{
			return LevelGrid::Current().ToWorld(column, row);
		}
	};
}

TEST_F(PathServiceTest, RouteLeadsFromOneRoomToAnother)
{
	std::vector<Vector2Df> route;

	ASSERT_TRUE(PathService::Current().RouteTo(At(1, 4), At(8, 1), route));
	EXPECT_FALSE(route.empty());

	int column = 0;
	int row = 0;
	LevelGrid::Current().ToCell(route.back(), column, row);
	EXPECT_EQ(column, 8);
	EXPECT_EQ(row, 1);
}

TEST_F(PathServiceTest, RouteGoesThroughTheDoorway)
{
	std::vector<Vector2Df> route;
	ASSERT_TRUE(PathService::Current().RouteTo(At(1, 4), At(8, 1), route));

	bool isDoorwayUsed = false;
	for (const Vector2Df& point : route)
	{
		int column = 0;
		int row = 0;
		LevelGrid::Current().ToCell(point, column, row);
		if (column == 5 && row == 3)
		{
			isDoorwayUsed = true;
		}
	}

	EXPECT_TRUE(isDoorwayUsed);
}

TEST_F(PathServiceTest, SameGoalIsBuiltOnce)
{
	std::vector<Vector2Df> route;
	PathService::Current().RouteTo(At(1, 4), At(8, 1), route);
	PathService::Current().RouteTo(At(3, 4), At(8, 1), route);
	PathService::Current().RouteTo(At(8, 2), At(8, 1), route);

	EXPECT_EQ(PathService::Current().GetBuildCount(), 1);
}

TEST_F(PathServiceTest, GoalInANewCellIsBuiltAgain)
{
	std::vector<Vector2Df> route;
	PathService::Current().RouteTo(At(1, 4), At(8, 1), route);
	PathService::Current().RouteTo(At(1, 4), At(7, 1), route);

	EXPECT_EQ(PathService::Current().GetBuildCount(), 2);
}

TEST_F(PathServiceTest, GoalsAreRememberedUpToTheSlotCount)
{
	std::vector<Vector2Df> route;
	for (int pass = 0; pass < 2; pass++)
	{
		for (int column = 1; column <= PATH_FIELD_SLOTS; column++)
		{
			PathService::Current().RouteTo(At(1, 4), At(column, 1), route);
		}
	}

	EXPECT_EQ(PathService::Current().GetBuildCount(), PATH_FIELD_SLOTS);
}

TEST_F(PathServiceTest, GoalInsideAWallGivesNoRoute)
{
	std::vector<Vector2Df> route;

	EXPECT_FALSE(PathService::Current().RouteTo(At(1, 4), At(0, 0), route));
	EXPECT_TRUE(route.empty());
}

TEST_F(PathServiceTest, StartInsideAWallGivesNoRoute)
{
	std::vector<Vector2Df> route;

	EXPECT_FALSE(PathService::Current().RouteTo(At(0, 0), At(8, 1), route));
	EXPECT_TRUE(route.empty());
}

TEST_F(PathServiceTest, WithoutALevelThereIsNoRoute)
{
	LevelGrid::SetCurrent(LevelGrid());
	PathService::Reset();

	std::vector<Vector2Df> route;
	EXPECT_FALSE(PathService::Current().RouteTo({0.f, 0.f}, {100.f, 100.f}, route));
	EXPECT_EQ(PathService::Current().FieldTo({100.f, 100.f}), nullptr);
}

TEST_F(PathServiceTest, ResetForgetsTheOldLevel)
{
	std::vector<Vector2Df> route;
	PathService::Current().RouteTo(At(1, 4), At(8, 1), route);
	ASSERT_EQ(PathService::Current().GetBuildCount(), 1);

	PathService::Reset();
	PathService::Current().RouteTo(At(1, 4), At(8, 1), route);

	EXPECT_EQ(PathService::Current().GetBuildCount(), 1);
}

TEST_F(PathServiceTest, FieldIsSharedBetweenChasers)
{
	const PathField* first = PathService::Current().FieldTo(At(8, 1));
	const PathField* second = PathService::Current().FieldTo(At(8, 1));

	ASSERT_NE(first, nullptr);
	EXPECT_EQ(first, second);
	EXPECT_EQ(PathService::Current().GetBuildCount(), 1);
}
