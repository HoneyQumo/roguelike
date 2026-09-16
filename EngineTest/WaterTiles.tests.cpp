#include "pch.h"
#include "Chasm.h"
#include "LevelGrid.h"
#include "LevelIntegrity.h"
#include "LevelLoader.h"
#include "TileAtlas.h"
#include <sstream>

using RoguelikeGame::IsChasmEdge;
using RoguelikeGame::LevelCell;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::LineFrame;
using RoguelikeGame::TileFrameFor;
using RoguelikeGame::TILE_FRAME_SIZE;
using RoguelikeGame::TILE_LINE_ACROSS;
using RoguelikeGame::TILE_LINE_ALONG;
using RoguelikeGame::TILE_LINE_ROW;
using RoguelikeGame::TILE_WATER_ROW;

namespace
{
	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "water");
	}

	// Полоса моста: отбойник, асфальт с разметкой посередине, вода за отбойником.
	const char* BRIDGE_STRIP =
		"[level]\n"
		"kind strip\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"~ Water\n"
		"- Line\n"
		"\n"
		"[map]\n"
		"~~~~~~\n"
		"######\n"
		"......\n"
		"------\n"
		"......\n"
		"######\n"
		"~~~~~~\n";

	const char* ALONG_STRIP =
		"[level]\n"
		"kind strip\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"- Line\n"
		"\n"
		"[map]\n"
		"#####\n"
		"..-..\n"
		"..-..\n"
		"..-..\n"
		"#####\n";
}

TEST(WaterTileTests, WaterAndLineAreReadFromTheLegend)
{
	LevelData level = LevelOf(BRIDGE_STRIP);

	EXPECT_EQ(level.tiles[0][0], RoguelikeGame::TileType::Water);
	EXPECT_EQ(level.tiles[3][0], RoguelikeGame::TileType::Line);
	EXPECT_EQ(level.tiles[2][0], RoguelikeGame::TileType::Floor);
}

TEST(WaterTileTests, WaterIsDrawnFromItsOwnRow)
{
	LevelData level = LevelOf(BRIDGE_STRIP);

	sf::IntRect frame = TileFrameFor(level, 0, 0);

	EXPECT_EQ(frame.top, TILE_WATER_ROW * TILE_FRAME_SIZE);
}

TEST(WaterTileTests, MarkingLiesAcrossItsOwnRow)
{
	LevelData level = LevelOf(BRIDGE_STRIP);

	sf::IntRect frame = TileFrameFor(level, 2, 3);

	EXPECT_EQ(frame.top, TILE_LINE_ROW * TILE_FRAME_SIZE);
	EXPECT_EQ(frame.left, TILE_LINE_ACROSS * TILE_FRAME_SIZE);
	EXPECT_EQ(LineFrame(level.tiles, 2, 3), TILE_LINE_ACROSS);
}

TEST(WaterTileTests, MarkingInAColumnTurnsAlongIt)
{
	LevelData level = LevelOf(ALONG_STRIP);

	EXPECT_EQ(LineFrame(level.tiles, 2, 2), TILE_LINE_ALONG);
}

TEST(WaterTileTests, YouCannotWalkOnWaterButYouCanWalkOnMarkings)
{
	LevelData level = LevelOf(BRIDGE_STRIP);
	LevelGrid grid = LevelGrid::Build(level);

	EXPECT_FALSE(grid.IsPassable(0, 0));
	EXPECT_TRUE(grid.IsPassable(0, 3));
	EXPECT_TRUE(grid.IsPassable(0, 2));
}

TEST(WaterTileTests, WaterNextToTheRoadGetsAnEdgeButWaterBehindTheRailDoesNot)
{
	LevelData level = LevelOf(BRIDGE_STRIP);

	// Между водой и дорогой стоит отбойник, лишние тела на кромке не нужны.
	EXPECT_FALSE(IsChasmEdge(level, 0, 0));

	LevelData open = LevelOf(
		"[level]\n"
		"kind strip\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"~ Water\n"
		"\n"
		"[map]\n"
		"#####\n"
		"#~~~#\n"
		"#...#\n"
		"#####\n");

	EXPECT_TRUE(IsChasmEdge(open, 2, 1));
}

TEST(WaterTileTests, TheValidatorTreatsWaterAsSolid)
{
	LevelData level = LevelOf(
		"[level]\n"
		"kind strip\n"
		"next city\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"~ Water\n"
		"@ PlayerSpawn\n"
		"> Exit\n"
		"\n"
		"[map]\n"
		"#######\n"
		"#@....#\n"
		"#.~~..#\n"
		"#....>#\n"
		"#######\n");

	RoguelikeGame::LevelReport report = RoguelikeGame::CheckLevel(level);

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}
