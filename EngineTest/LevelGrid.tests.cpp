#include "pch.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include <sstream>

using RoguelikeGame::LevelCell;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
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

TEST(LevelGridTest, EmptyGridHidesNothing)
{
	LevelGrid grid;

	EXPECT_TRUE(grid.IsEmpty());
	EXPECT_FALSE(grid.HasWallBetween({0.f, 0.f}, {1000.f, 1000.f}));
}
