#include "pch.h"
#include "LevelLoader.h"
#include <sstream>
#include <stdexcept>

using namespace RoguelikeGame;

namespace
{
	LevelData ParseLevel(const std::string& text)
	{
		std::istringstream input(text);
		return LevelLoader::Parse(input, "test");
	}
}

TEST(LevelLoaderTests, ParsesDefaultLegendWithoutLegendSection)
{
	LevelData level = ParseLevel("[map]\n###\n#@#\n###\n");

	EXPECT_EQ(level.width, 3);
	EXPECT_EQ(level.height, 3);
	EXPECT_EQ(CountTiles(level, TileType::Wall), 8);
	EXPECT_EQ(CountTiles(level, TileType::PlayerSpawn), 1);
}

TEST(LevelLoaderTests, CustomLegendOverridesDefault)
{
	LevelData level = ParseLevel("[legend]\nX Wall\no Floor\n[map]\nXXX\nXoX\nXXX\n");

	EXPECT_EQ(CountTiles(level, TileType::Wall), 8);
	EXPECT_EQ(CountTiles(level, TileType::Floor), 1);
}

TEST(LevelLoaderTests, WidthIsTakenFromLongestRow)
{
	LevelData level = ParseLevel("[map]\n#\n####\n##\n");

	EXPECT_EQ(level.width, 4);
	EXPECT_EQ(level.height, 3);
}

TEST(LevelLoaderTests, CommentsAndBlankLinesAreSkipped)
{
	LevelData level = ParseLevel("; leading comment\n\n[map]\n###\n\n; another comment\n###\n");

	EXPECT_EQ(level.height, 2);
}

TEST(LevelLoaderTests, EmptyLevelIsRejected)
{
	EXPECT_THROW(ParseLevel("[map]\n"), std::runtime_error);
}

TEST(LevelLoaderTests, UnknownTileTypeInLegendIsRejected)
{
	EXPECT_THROW(ParseLevel("[legend]\nX NoSuchTile\n[map]\nX\n"), std::runtime_error);
}

TEST(LevelLoaderTests, LegendLineWithoutNameIsRejected)
{
	EXPECT_THROW(ParseLevel("[legend]\nX\n[map]\nX\n"), std::runtime_error);
}

TEST(LevelLoaderTests, EveryEnemySpawnSymbolIsRecognised)
{
	LevelData level = ParseLevel("[map]\ngashrb\n");

	EXPECT_EQ(CountTiles(level, TileType::GruntSpawn), 1);
	EXPECT_EQ(CountTiles(level, TileType::AssaultSpawn), 1);
	EXPECT_EQ(CountTiles(level, TileType::ShieldSpawn), 1);
	EXPECT_EQ(CountTiles(level, TileType::HeavySpawn), 1);
	EXPECT_EQ(CountTiles(level, TileType::RadioSpawn), 1);
	EXPECT_EQ(CountTiles(level, TileType::BossSpawn), 1);
}

TEST(LevelLoaderTests, SecondPlayerSpawnStaysInTheData)
{
	LevelData level = ParseLevel("[map]\n@.@\n");

	EXPECT_EQ(CountTiles(level, TileType::PlayerSpawn), 2);
}

TEST(LevelLoaderTests, LevelWithoutPlayerSpawnHasNoSpawnTiles)
{
	LevelData level = ParseLevel("[map]\n###\n...\n");

	EXPECT_EQ(CountTiles(level, TileType::PlayerSpawn), 0);
	EXPECT_EQ(CountTiles(level, TileType::Wall), 3);
}

TEST(LevelLoaderTests, MissingFileThrows)
{
	EXPECT_THROW(LevelLoader::Load("no_such_level.config"), std::runtime_error);
}
