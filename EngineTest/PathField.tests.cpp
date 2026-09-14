#include "pch.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "PathField.h"
#include <sstream>
#include <chrono>

using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PathField;
using RoguelikeGame::PATH_UNREACHABLE;
using XYZEngine::Vector2Df;

namespace
{
	LevelGrid GridOf(const std::string& map)
	{
		std::istringstream input(map);
		LevelData level = LevelLoader::Parse(input, "path");

		return LevelGrid::Build(level);
	}

	const std::string SNAKE =
		"[map]\n"
		"#######\n"
		"#.....#\n"
		"#####.#\n"
		"#.....#\n"
		"#.#####\n"
		"#.....#\n"
		"#######\n";

	const std::string TWO_ROOMS =
		"[map]\n"
		"#######\n"
		"#..#..#\n"
		"#..#..#\n"
		"#######\n";
}

TEST(PathFieldTest, SnakeCorridorIsWalkedEndToEnd)
{
	LevelGrid grid = GridOf(SNAKE);
	PathField field;
	field.Build(grid, 1, 5);

	ASSERT_TRUE(field.IsReachable(1, 1));

	std::vector<Vector2Df> route;
	ASSERT_TRUE(field.BuildRoute(grid, 1, 1, route));

	EXPECT_EQ(route.size(), static_cast<std::size_t>(field.GetDistance(1, 1)));
	EXPECT_EQ(route.back().x, grid.ToWorld(1, 5).x);
	EXPECT_EQ(route.back().y, grid.ToWorld(1, 5).y);
}

TEST(PathFieldTest, EveryStepOfTheRouteStaysOnFloor)
{
	LevelGrid grid = GridOf(SNAKE);
	PathField field;
	field.Build(grid, 1, 5);

	std::vector<Vector2Df> route;
	ASSERT_TRUE(field.BuildRoute(grid, 1, 1, route));

	for (const Vector2Df& point : route)
	{
		int column = 0;
		int row = 0;
		grid.ToCell(point, column, row);

		EXPECT_TRUE(grid.IsPassable(column, row)) << "cell " << column << ":" << row;
	}
}

TEST(PathFieldTest, RouteGoesAroundTheWallNotThroughIt)
{
	LevelGrid grid = GridOf(SNAKE);
	PathField field;
	field.Build(grid, 1, 5);

	std::vector<Vector2Df> route;
	ASSERT_TRUE(field.BuildRoute(grid, 1, 1, route));

	EXPECT_GT(route.size(), 4u);
}

TEST(PathFieldTest, WalledOffRoomIsNotReachable)
{
	LevelGrid grid = GridOf(TWO_ROOMS);
	PathField field;
	field.Build(grid, 1, 1);

	EXPECT_TRUE(field.IsReachable(2, 2));
	EXPECT_FALSE(field.IsReachable(5, 1));
	EXPECT_EQ(field.GetDistance(5, 1), PATH_UNREACHABLE);

	std::vector<Vector2Df> route;
	EXPECT_FALSE(field.BuildRoute(grid, 5, 1, route));
	EXPECT_TRUE(route.empty());
}

TEST(PathFieldTest, GoalInsideAWallGivesNoField)
{
	LevelGrid grid = GridOf(TWO_ROOMS);
	PathField field;
	field.Build(grid, 3, 1);

	EXPECT_TRUE(field.IsEmpty());
	EXPECT_FALSE(field.IsReachable(1, 1));
}

TEST(PathFieldTest, StandingOnTheGoalNeedsNoRoute)
{
	LevelGrid grid = GridOf(SNAKE);
	PathField field;
	field.Build(grid, 1, 5);

	EXPECT_EQ(field.GetDistance(1, 5), 0);

	std::vector<Vector2Df> route;
	EXPECT_FALSE(field.BuildRoute(grid, 1, 5, route));
}

TEST(PathFieldTest, DistanceGrowsByOnePerStep)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"#######\n"
		"#.....#\n"
		"#######\n");

	PathField field;
	field.Build(grid, 1, 1);

	EXPECT_EQ(field.GetDistance(1, 1), 0);
	EXPECT_EQ(field.GetDistance(2, 1), 1);
	EXPECT_EQ(field.GetDistance(5, 1), 4);
}

TEST(PathFieldTest, RouteInAnOpenRoomIsAsShortAsTheDistance)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"########\n"
		"#......#\n"
		"#......#\n"
		"#......#\n"
		"#......#\n"
		"########\n");

	PathField field;
	field.Build(grid, 6, 4);

	std::vector<Vector2Df> route;
	ASSERT_TRUE(field.BuildRoute(grid, 1, 1, route));

	EXPECT_EQ(route.size(), 8u);
	EXPECT_EQ(route.size(), static_cast<std::size_t>(field.GetDistance(1, 1)));
}

TEST(PathFieldTest, EveryStepOfTheRouteGetsCloser)
{
	LevelGrid grid = GridOf(SNAKE);
	PathField field;
	field.Build(grid, 1, 5);

	std::vector<Vector2Df> route;
	ASSERT_TRUE(field.BuildRoute(grid, 1, 1, route));

	int previous = field.GetDistance(1, 1);
	for (const Vector2Df& point : route)
	{
		int column = 0;
		int row = 0;
		grid.ToCell(point, column, row);

		EXPECT_EQ(field.GetDistance(column, row), previous - 1);
		previous--;
	}

	EXPECT_EQ(previous, 0);
}

TEST(PathFieldTest, RaggedRowsAreImpassable)
{
	LevelGrid grid = GridOf("[map]\n#####\n#..\n#...#\n#####\n");

	PathField field;
	field.Build(grid, 1, 1);

	EXPECT_FALSE(field.IsReachable(3, 1));
	EXPECT_TRUE(field.IsReachable(3, 2));
}

TEST(PathFieldTest, EmptyGridGivesNoField)
{
	LevelGrid grid;
	PathField field;
	field.Build(grid, 0, 0);

	EXPECT_TRUE(field.IsEmpty());

	std::vector<Vector2Df> route;
	EXPECT_FALSE(field.BuildRoute(grid, 0, 0, route));
}

TEST(PathFieldTest, ClearedFieldForgetsEverything)
{
	LevelGrid grid = GridOf(SNAKE);
	PathField field;
	field.Build(grid, 1, 5);
	ASSERT_FALSE(field.IsEmpty());

	field.Clear();

	EXPECT_TRUE(field.IsEmpty());
	EXPECT_FALSE(field.IsReachable(1, 1));
}

TEST(PathFieldTest, BenchmarkOnActSizedGrid)
{
	constexpr int COLUMNS = 64;
	constexpr int ROWS = 38;
	constexpr int BUILDS = 200;

	std::string map = "[map]\n";
	for (int row = 0; row < ROWS; row++)
	{
		for (int column = 0; column < COLUMNS; column++)
		{
			bool isWall = row == 0 || column == 0 || row == ROWS - 1 || column == COLUMNS - 1
				|| (row % 6 == 0 && column % 11 != 5);
			map += isWall ? '#' : '.';
		}
		map += '\n';
	}

	LevelGrid grid = GridOf(map);
	ASSERT_EQ(grid.GetWidth(), COLUMNS);
	ASSERT_EQ(grid.GetHeight(), ROWS);

	PathField field;
	auto started = std::chrono::steady_clock::now();
	for (int build = 0; build < BUILDS; build++)
	{
		field.Build(grid, 1 + build % 40, 1);
	}
	auto elapsed = std::chrono::steady_clock::now() - started;

	ASSERT_TRUE(field.IsReachable(1, ROWS - 3));

	std::cout << "PathField::Build on " << (COLUMNS * ROWS) << " cells: "
		<< std::chrono::duration<double, std::micro>(elapsed).count() / BUILDS << " us per build" << std::endl;
}
