#include "pch.h"
#include "LevelLoader.h"
#include "TileAtlas.h"
#include <set>
#include <sstream>

using RoguelikeGame::FloorFrame;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::TileFrameFor;
using RoguelikeGame::TileFrameRect;
using RoguelikeGame::WallFrame;
using RoguelikeGame::WallMask;

namespace
{
	LevelData ParseLevel(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "tiles");
	}

	constexpr int UP = RoguelikeGame::WALL_NEIGHBOUR_UP;
	constexpr int RIGHT = RoguelikeGame::WALL_NEIGHBOUR_RIGHT;
	constexpr int DOWN = RoguelikeGame::WALL_NEIGHBOUR_DOWN;
	constexpr int LEFT = RoguelikeGame::WALL_NEIGHBOUR_LEFT;
}

TEST(TileAtlasTest, WallInsideAWallBlockSeesEveryNeighbour)
{
	LevelData level = ParseLevel("[map]\n###\n###\n###\n");

	EXPECT_EQ(WallMask(level, 1, 1), UP | RIGHT | DOWN | LEFT);
}

TEST(TileAtlasTest, FloorNextToAWallTakesTheEdgeAway)
{
	LevelData level = ParseLevel(
		"[map]\n"
		"#####\n"
		"#...#\n"
		"#.#.#\n"
		"#...#\n"
		"#####\n");

	EXPECT_EQ(WallMask(level, 2, 2), 0);
	EXPECT_EQ(WallMask(level, 1, 0), UP | RIGHT | LEFT);
	EXPECT_EQ(WallMask(level, 0, 0), UP | RIGHT | DOWN | LEFT);
	EXPECT_EQ(WallMask(level, 0, 2), UP | DOWN | LEFT);
}

TEST(TileAtlasTest, CornerAndCorridorReadDifferently)
{
	LevelData level = ParseLevel(
		"[map]\n"
		".....\n"
		".###.\n"
		".#...\n"
		".#...\n"
		".....\n");

	int corner = WallMask(level, 1, 1);
	int corridor = WallMask(level, 1, 2);
	int edge = WallMask(level, 2, 1);

	EXPECT_EQ(corner, RIGHT | DOWN);
	EXPECT_EQ(corridor, UP | DOWN);
	EXPECT_EQ(edge, RIGHT | LEFT);
	EXPECT_NE(WallFrame(corner), WallFrame(corridor));
	EXPECT_NE(WallFrame(corner), WallFrame(edge));
}

TEST(TileAtlasTest, OutsideTheMapCountsAsWall)
{
	LevelData level = ParseLevel("[map]\n#\n");

	EXPECT_EQ(WallMask(level, 0, 0), UP | RIGHT | DOWN | LEFT);
}

TEST(TileAtlasTest, RaggedRowDoesNotReadPastItsEnd)
{
	LevelData level = ParseLevel("[map]\n####\n#\n####\n");

	EXPECT_EQ(WallMask(level, 3, 1), UP | RIGHT | DOWN | LEFT);
	EXPECT_EQ(WallMask(level, 0, 1), UP | RIGHT | DOWN | LEFT);
}

TEST(TileAtlasTest, WallFrameStaysInsideTheRow)
{
	for (int mask = 0; mask < 32; mask++)
	{
		EXPECT_GE(WallFrame(mask), 0);
		EXPECT_LT(WallFrame(mask), RoguelikeGame::TILE_WALL_FRAMES);
	}

	EXPECT_EQ(WallFrame(-1), 0);
}

TEST(TileAtlasTest, FloorFrameIsTheSameEveryRun)
{
	for (int row = 0; row < 20; row++)
	{
		for (int column = 0; column < 20; column++)
		{
			int frame = FloorFrame(column, row);

			EXPECT_EQ(frame, FloorFrame(column, row));
			EXPECT_GE(frame, 0);
			EXPECT_LT(frame, RoguelikeGame::TILE_FLOOR_FRAMES);
		}
	}
}

TEST(TileAtlasTest, FloorUsesEveryVariant)
{
	std::set<int> seen;

	for (int row = 0; row < 25; row++)
	{
		for (int column = 0; column < 33; column++)
		{
			seen.insert(FloorFrame(column, row));
		}
	}

	EXPECT_EQ(static_cast<int>(seen.size()), RoguelikeGame::TILE_FLOOR_FRAMES);
}

TEST(TileAtlasTest, FrameRectStaysInsideTheAtlas)
{
	for (int row = 0; row < 2; row++)
	{
		for (int column = 0; column < RoguelikeGame::TILE_ATLAS_COLUMNS; column++)
		{
			sf::IntRect rect = TileFrameRect(row, column);

			EXPECT_GE(rect.left, 0);
			EXPECT_GE(rect.top, 0);
			EXPECT_LE(rect.left + rect.width, RoguelikeGame::TILE_ATLAS_COLUMNS * RoguelikeGame::TILE_FRAME_SIZE);
			EXPECT_LE(rect.top + rect.height, 2 * RoguelikeGame::TILE_FRAME_SIZE);
		}
	}
}

TEST(TileAtlasTest, WallAndFloorComeFromDifferentRows)
{
	LevelData level = ParseLevel("[map]\n###\n#.#\n###\n");

	sf::IntRect wall = TileFrameFor(level, 0, 0);
	sf::IntRect floor = TileFrameFor(level, 1, 1);

	EXPECT_EQ(wall.top, RoguelikeGame::TILE_WALL_ROW * RoguelikeGame::TILE_FRAME_SIZE);
	EXPECT_EQ(floor.top, RoguelikeGame::TILE_FLOOR_ROW * RoguelikeGame::TILE_FRAME_SIZE);
}
