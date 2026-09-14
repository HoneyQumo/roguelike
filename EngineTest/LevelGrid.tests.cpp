#include "pch.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include <sstream>

using RoguelikeGame::LevelCell;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::TILE_SIZE;
using XYZEngine::Vector2Df;

namespace
{
	LevelGrid GridOf(const std::string& map)
	{
		std::istringstream input(map);
		LevelData level = LevelLoader::Parse(input, "grid");

		return LevelGrid::Build(level);
	}

	const std::string ROOMS =
		"[map]\n"
		"#####\n"
		"#...#\n"
		"##.##\n"
		"#...#\n"
		"#####\n";
}

TEST(LevelGridTest, SizeComesFromTheMap)
{
	LevelGrid grid = GridOf(ROOMS);

	EXPECT_EQ(grid.GetWidth(), 5);
	EXPECT_EQ(grid.GetHeight(), 5);
	EXPECT_FALSE(grid.IsEmpty());
}

TEST(LevelGridTest, WallsAndFloorAreToldApart)
{
	LevelGrid grid = GridOf(ROOMS);

	EXPECT_TRUE(grid.IsPassable(2, 1));
	EXPECT_FALSE(grid.IsPassable(0, 0));
	EXPECT_TRUE(grid.BlocksSight(0, 0));
	EXPECT_FALSE(grid.BlocksSight(2, 1));
}

TEST(LevelGridTest, OutsideTheMapBlocksSight)
{
	LevelGrid grid = GridOf(ROOMS);

	EXPECT_TRUE(grid.BlocksSight(-1, 0));
	EXPECT_TRUE(grid.BlocksSight(0, 99));
	EXPECT_FALSE(grid.IsPassable(-1, 0));
}

TEST(LevelGridTest, ShortRowEndsInOutside)
{
	LevelGrid grid = GridOf("[map]\n####\n#\n####\n");

	EXPECT_EQ(grid.GetWidth(), 4);
	EXPECT_EQ(grid.GetCell(0, 1), LevelCell::Wall);
	EXPECT_EQ(grid.GetCell(3, 1), LevelCell::Outside);
	EXPECT_TRUE(grid.BlocksSight(3, 1));
}

TEST(LevelGridTest, GapDoesNotBlockSight)
{
	LevelGrid grid = GridOf("[map]\n###\n# #\n###\n");

	EXPECT_EQ(grid.GetCell(1, 1), LevelCell::Gap);
	EXPECT_FALSE(grid.BlocksSight(1, 1));
	EXPECT_FALSE(grid.IsPassable(1, 1));
}

TEST(LevelGridTest, CellAndWorldAgree)
{
	LevelGrid grid = GridOf(ROOMS);

	for (int row = 0; row < grid.GetHeight(); row++)
	{
		for (int column = 0; column < grid.GetWidth(); column++)
		{
			Vector2Df world = grid.ToWorld(column, row);
			int backColumn = -1;
			int backRow = -1;
			grid.ToCell(world, backColumn, backRow);

			EXPECT_EQ(backColumn, column);
			EXPECT_EQ(backRow, row);
		}
	}
}

TEST(LevelGridTest, WallBetweenRoomsIsFound)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"#######\n"
		"#..#..#\n"
		"#..#..#\n"
		"#######\n");

	Vector2Df left = grid.ToWorld(1, 1);
	Vector2Df right = grid.ToWorld(5, 1);

	EXPECT_TRUE(grid.HasWallBetween(left, right));
}

TEST(LevelGridTest, OpenRoomHasNoWallBetween)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"#######\n"
		"#.....#\n"
		"#.....#\n"
		"#######\n");

	EXPECT_FALSE(grid.HasWallBetween(grid.ToWorld(1, 1), grid.ToWorld(5, 1)));
	EXPECT_FALSE(grid.HasWallBetween(grid.ToWorld(1, 1), grid.ToWorld(5, 2)));
}

TEST(LevelGridTest, NeighbourCellsAreAlwaysVisible)
{
	LevelGrid grid = GridOf(ROOMS);

	EXPECT_FALSE(grid.HasWallBetween(grid.ToWorld(1, 1), grid.ToWorld(2, 1)));
	EXPECT_FALSE(grid.HasWallBetween(grid.ToWorld(1, 1), grid.ToWorld(1, 1)));
}

TEST(LevelGridTest, CornerOfAWallIsNotSeenThrough)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"#########\n"
		"#.......#\n"
		"#.......#\n"
		"#.#####.#\n"
		"#.......#\n"
		"#########\n");

	Vector2Df below = grid.ToWorld(1, 4);
	Vector2Df above = grid.ToWorld(2, 1);

	EXPECT_TRUE(grid.HasWallBetween(below, above));
	EXPECT_TRUE(grid.HasWallBetween({below.x + 30.f, below.y}, {above.x - 28.f, above.y}));
}

TEST(LevelGridTest, WallAcrossTheRowBlocksTheView)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"#####\n"
		"#...#\n"
		"#.#.#\n"
		"#...#\n"
		"#####\n");

	Vector2Df left = grid.ToWorld(1, 2);
	Vector2Df right = grid.ToWorld(3, 2);

	EXPECT_TRUE(grid.HasWallBetween(left, right));
	EXPECT_FALSE(grid.HasWallBetween({left.x, left.y + TILE_SIZE}, {right.x, right.y + TILE_SIZE}));
}

TEST(LevelGridTest, WallRowWithAGapBlocksEverywhereButTheGap)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"##########\n"
		"#........#\n"
		"#####.####\n"
		"#........#\n"
		"##########\n");

	EXPECT_TRUE(grid.HasWallBetween(grid.ToWorld(1, 1), grid.ToWorld(8, 3)));
	EXPECT_FALSE(grid.HasWallBetween(grid.ToWorld(5, 1), grid.ToWorld(5, 3)));
}

TEST(LevelGridTest, DiagonalCornerCutIsBlocked)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"#####\n"
		"#.#.#\n"
		"##.##\n"
		"#.#.#\n"
		"#####\n");

	EXPECT_TRUE(grid.HasWallBetween(grid.ToWorld(1, 1), grid.ToWorld(3, 3)));
}

TEST(LevelGridTest, CellBehindTheTargetIsNotChecked)
{
	LevelGrid grid = GridOf(ROOMS);

	EXPECT_FALSE(grid.HasWallBetween(grid.ToWorld(2, 1), grid.ToWorld(2, 0)));
}

TEST(LevelGridTest, CrateCellIsNotWalkableButIsSeenThrough)
{
	LevelGrid grid = GridOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"c Prop:crate_ammo\n"
		"[map]\n"
		"#####\n"
		"#.c.#\n"
		"#...#\n"
		"#####\n");

	EXPECT_EQ(grid.GetCell(2, 1), RoguelikeGame::LevelCell::Blocked);
	EXPECT_FALSE(grid.IsPassable(2, 1));
	EXPECT_FALSE(grid.BlocksSight(2, 1));
}

TEST(LevelGridTest, CrateBlocksTheWayButNotTheView)
{
	LevelGrid grid = GridOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"c Prop:crate_ammo\n"
		"[map]\n"
		"#####\n"
		"#.c.#\n"
		"#...#\n"
		"#####\n");

	Vector2Df left = grid.ToWorld(1, 1);
	Vector2Df right = grid.ToWorld(3, 1);

	EXPECT_FALSE(grid.HasWallBetween(left, right));
	EXPECT_TRUE(grid.HasObstacleBetween(left, right));
}

TEST(LevelGridTest, BrokenCrateOpensTheCellAgain)
{
	LevelGrid::SetCurrent(GridOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"c Prop:crate_ammo\n"
		"[map]\n"
		"#####\n"
		"#.c.#\n"
		"#...#\n"
		"#####\n"));

	Vector2Df left = LevelGrid::Current().ToWorld(1, 1);
	Vector2Df right = LevelGrid::Current().ToWorld(3, 1);
	ASSERT_TRUE(LevelGrid::Current().HasObstacleBetween(left, right));

	LevelGrid::OpenCell(LevelGrid::Current().ToWorld(2, 1));

	EXPECT_TRUE(LevelGrid::Current().IsPassable(2, 1));
	EXPECT_FALSE(LevelGrid::Current().HasObstacleBetween(left, right));

	LevelGrid::SetCurrent(LevelGrid());
}

TEST(LevelGridTest, OpeningAWallChangesNothing)
{
	LevelGrid::SetCurrent(GridOf(ROOMS));
	ASSERT_FALSE(LevelGrid::Current().IsPassable(0, 0));

	LevelGrid::OpenCell(LevelGrid::Current().ToWorld(0, 0));

	EXPECT_FALSE(LevelGrid::Current().IsPassable(0, 0));

	LevelGrid::SetCurrent(LevelGrid());
}

TEST(LevelGridTest, WallStopsBothTheViewAndTheWay)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"#####\n"
		"#.#.#\n"
		"#...#\n"
		"#####\n");

	Vector2Df left = grid.ToWorld(1, 1);
	Vector2Df right = grid.ToWorld(3, 1);

	EXPECT_TRUE(grid.HasWallBetween(left, right));
	EXPECT_TRUE(grid.HasObstacleBetween(left, right));
}

TEST(LevelGridTest, FreeSpotIsTheCellItselfWhenItIsFree)
{
	LevelGrid grid = GridOf(ROOMS);

	Vector2Df spot = {0.f, 0.f};
	ASSERT_TRUE(grid.FindFreeSpot(grid.ToWorld(1, 1), spot));

	EXPECT_EQ(spot.x, grid.ToWorld(1, 1).x);
	EXPECT_EQ(spot.y, grid.ToWorld(1, 1).y);
}

TEST(LevelGridTest, FreeSpotIsFoundNextToAWall)
{
	LevelGrid grid = GridOf(ROOMS);

	Vector2Df spot = {0.f, 0.f};
	ASSERT_TRUE(grid.FindFreeSpot(grid.ToWorld(0, 0), spot));

	int column = 0;
	int row = 0;
	grid.ToCell(spot, column, row);

	EXPECT_TRUE(grid.IsPassable(column, row));
}

TEST(LevelGridTest, FreeSpotIsFoundNextToACrate)
{
	LevelGrid grid = GridOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"c Prop:crate_ammo\n"
		"[map]\n"
		"#####\n"
		"#.c.#\n"
		"#...#\n"
		"#####\n");

	Vector2Df spot = {0.f, 0.f};
	ASSERT_TRUE(grid.FindFreeSpot(grid.ToWorld(2, 1), spot));

	int column = 0;
	int row = 0;
	grid.ToCell(spot, column, row);

	EXPECT_TRUE(grid.IsPassable(column, row));
}

TEST(LevelGridTest, WithoutALevelThereIsNoFreeSpot)
{
	LevelGrid grid;

	Vector2Df spot = {0.f, 0.f};
	EXPECT_FALSE(grid.FindFreeSpot({0.f, 0.f}, spot));
}

TEST(LevelGridTest, EmptyGridHidesNothing)
{
	LevelGrid grid;

	EXPECT_TRUE(grid.IsEmpty());
	EXPECT_FALSE(grid.HasWallBetween({0.f, 0.f}, {1000.f, 1000.f}));
}
