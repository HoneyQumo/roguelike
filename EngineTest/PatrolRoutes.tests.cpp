#include "pch.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "PatrolRoutes.h"
#include <sstream>

using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PatrolRoute;
using RoguelikeGame::PatrolRoutes;
using RoguelikeGame::TILE_SIZE;
using XYZEngine::Vector2Df;

namespace
{
	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "patrol");
	}

	const std::string ONE_ROUTE =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"1 Patrol:watch\n"
		"2 Patrol:watch\n"
		"3 Patrol:watch\n"
		"[map]\n"
		"#######\n"
		"#3...2#\n"
		"#.....#\n"
		"#@...1#\n"
		"#######\n";

	const std::string TWO_ROUTES =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"a Patrol:north\n"
		"b Patrol:north\n"
		"c Patrol:south\n"
		"d Patrol:south\n"
		"[map]\n"
		"#######\n"
		"#a...b#\n"
		"#..@..#\n"
		"#c...d#\n"
		"#######\n";
}

TEST(PatrolRoutesTest, PointsAreReadFromTheMap)
{
	LevelData level = LevelOf(ONE_ROUTE);

	EXPECT_EQ(level.patrols.size(), 3u);
}

TEST(PatrolRoutesTest, RouteKeepsTheLegendOrder)
{
	LevelData level = LevelOf(ONE_ROUTE);
	LevelGrid grid = LevelGrid::Build(level);
	PatrolRoutes routes = PatrolRoutes::Build(level, grid);

	const PatrolRoute* route = routes.Find("watch");
	ASSERT_NE(route, nullptr);
	ASSERT_EQ(route->points.size(), 3u);

	EXPECT_EQ(route->points[0].position.y, grid.ToWorld(5, 3).y);
	EXPECT_EQ(route->points[1].position.y, grid.ToWorld(5, 1).y);
	EXPECT_EQ(route->points[2].position.y, grid.ToWorld(1, 1).y);
	EXPECT_EQ(route->points[2].position.x, grid.ToWorld(1, 1).x);
}

TEST(PatrolRoutesTest, RoutesAreToldApart)
{
	LevelData level = LevelOf(TWO_ROUTES);
	LevelGrid grid = LevelGrid::Build(level);
	PatrolRoutes routes = PatrolRoutes::Build(level, grid);

	EXPECT_EQ(routes.GetCount(), 2u);
	ASSERT_NE(routes.Find("north"), nullptr);
	ASSERT_NE(routes.Find("south"), nullptr);
	EXPECT_EQ(routes.Find("north")->points.size(), 2u);
	EXPECT_EQ(routes.Find("south")->points.size(), 2u);
	EXPECT_EQ(routes.Find("nowhere"), nullptr);
}

TEST(PatrolRoutesTest, NearestRouteIsPickedForTheEnemy)
{
	LevelData level = LevelOf(TWO_ROUTES);
	LevelGrid grid = LevelGrid::Build(level);
	PatrolRoutes routes = PatrolRoutes::Build(level, grid);

	const PatrolRoute* closer = routes.Nearest(grid.ToWorld(1, 1), 4.f * TILE_SIZE);
	ASSERT_NE(closer, nullptr);
	EXPECT_EQ(closer->id, "north");

	const PatrolRoute* farther = routes.Nearest(grid.ToWorld(1, 3), 4.f * TILE_SIZE);
	ASSERT_NE(farther, nullptr);
	EXPECT_EQ(farther->id, "south");
}

TEST(PatrolRoutesTest, RouteTooFarAwayIsNotJoined)
{
	LevelData level = LevelOf(TWO_ROUTES);
	LevelGrid grid = LevelGrid::Build(level);
	PatrolRoutes routes = PatrolRoutes::Build(level, grid);

	EXPECT_EQ(routes.Nearest({10000.f, 10000.f}, 4.f * TILE_SIZE), nullptr);
}

TEST(PatrolRoutesTest, LevelWithoutPatrolsHasNoRoutes)
{
	LevelData level = LevelOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"[map]\n"
		"#####\n"
		"#...#\n"
		"#.@.#\n"
		"#####\n");
	LevelGrid grid = LevelGrid::Build(level);
	PatrolRoutes routes = PatrolRoutes::Build(level, grid);

	EXPECT_TRUE(routes.IsEmpty());
	EXPECT_EQ(routes.Nearest({0.f, 0.f}, 1000.f), nullptr);
}

TEST(PatrolRoutesTest, PatrolPointIsWalkableFloor)
{
	LevelData level = LevelOf(ONE_ROUTE);
	LevelGrid grid = LevelGrid::Build(level);

	for (const RoguelikeGame::PatrolPoint& point : level.patrols)
	{
		EXPECT_TRUE(grid.IsPassable(point.column, point.row)) << point.routeId;
	}
}

TEST(PatrolRoutesTest, CurrentRoutesAreShared)
{
	LevelData level = LevelOf(ONE_ROUTE);
	LevelGrid grid = LevelGrid::Build(level);
	PatrolRoutes::SetCurrent(PatrolRoutes::Build(level, grid));

	EXPECT_EQ(PatrolRoutes::Current().GetCount(), 1u);

	PatrolRoutes::SetCurrent(PatrolRoutes());
	EXPECT_TRUE(PatrolRoutes::Current().IsEmpty());
}
