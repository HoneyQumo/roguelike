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

TEST(LevelLoaderTests, ItemLegendEntryProducesPlacement)
{
	std::istringstream input(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"k Item:key_rusty\n"
		"@ PlayerSpawn\n"
		"[map]\n"
		"#k@#\n");

	LevelData level = LevelLoader::Parse(input, "test");

	ASSERT_EQ(level.items.size(), 1u);
	EXPECT_EQ(level.items[0].itemId, "key_rusty");
	EXPECT_EQ(level.items[0].column, 1);
	EXPECT_EQ(level.items[0].row, 0);
	EXPECT_EQ(level.tiles[0][1], TileType::Floor);
}

TEST(LevelLoaderTests, ItemPlacementsKeepTheirCoordinates)
{
	std::istringstream input(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"p Item:potion_small\n"
		"w Item:weapon_deagle\n"
		"[map]\n"
		"#p.#\n"
		"#.w@\n");

	LevelData level = LevelLoader::Parse(input, "test");

	ASSERT_EQ(level.items.size(), 2u);
	EXPECT_EQ(level.items[0].itemId, "potion_small");
	EXPECT_EQ(level.items[0].row, 0);
	EXPECT_EQ(level.items[1].itemId, "weapon_deagle");
	EXPECT_EQ(level.items[1].column, 2);
	EXPECT_EQ(level.items[1].row, 1);
}

TEST(LevelLoaderTests, ItemLegendWithoutIdIsRejected)
{
	std::istringstream input(
		"[legend]\n"
		"k Item:\n"
		"[map]\n"
		"#k#\n");

	EXPECT_THROW(LevelLoader::Parse(input, "test"), std::runtime_error);
}

TEST(LevelLoaderTests, LoaderDoesNotValidateItemIds)
{
	std::istringstream input(
		"[legend]\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"x Item:no_such_item\n"
		"[map]\n"
		".x@\n");

	LevelData level = LevelLoader::Parse(input, "test");

	ASSERT_EQ(level.items.size(), 1u);
	EXPECT_EQ(level.items[0].itemId, "no_such_item");
}

TEST(LevelLoaderTests, LevelSectionCarriesTitleNextAndBoss)
{
	std::istringstream input(
		"[level]\n"
		"title Gorod\n"
		"next arena\n"
		"boss Boss 2.5 1.5\n"
		"[legend]\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"> Exit\n"
		"[map]\n"
		".@>\n");

	LevelData level = LevelLoader::Parse(input, "test");

	EXPECT_EQ(level.info.title, "Gorod");
	EXPECT_EQ(level.info.nextLevelId, "arena");
	EXPECT_EQ(level.info.boss.bossId, "Boss");
	EXPECT_FLOAT_EQ(level.info.boss.healthScale, 2.5f);
	EXPECT_FLOAT_EQ(level.info.boss.damageScale, 1.5f);
}

TEST(LevelLoaderTests, EntranceAndExitAreRegularTiles)
{
	std::istringstream input(
		"[legend]\n"
		". Floor\n"
		"< Entrance\n"
		"> Exit\n"
		"[map]\n"
		"<.>\n");

	LevelData level = LevelLoader::Parse(input, "test");

	EXPECT_EQ(level.tiles[0][0], TileType::Entrance);
	EXPECT_EQ(level.tiles[0][2], TileType::Exit);
}

TEST(LevelLoaderTests, NextLevelWithoutExitIsRejected)
{
	std::istringstream input(
		"[level]\n"
		"next arena\n"
		"[legend]\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"[map]\n"
		".@.\n");

	EXPECT_THROW(LevelLoader::Parse(input, "test"), std::runtime_error);
}

TEST(LevelLoaderTests, BossWithoutNameIsRejected)
{
	std::istringstream input(
		"[level]\n"
		"boss\n"
		"[legend]\n"
		"@ PlayerSpawn\n"
		"[map]\n"
		"@\n");

	EXPECT_THROW(LevelLoader::Parse(input, "test"), std::runtime_error);
}

TEST(LevelLoaderTests, EntranceWorksAsStartPoint)
{
	std::istringstream input(
		"[legend]\n"
		". Floor\n"
		"< Entrance\n"
		"[map]\n"
		".<.\n");

	LevelData level = LevelLoader::Parse(input, "test");

	EXPECT_EQ(level.tiles[0][1], TileType::Entrance);
}

TEST(LevelLoaderTests, DoorInTheLegendBecomesADoorTile)
{
	std::istringstream input(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"+ Door:door_exit\n"
		"[map]\n"
		"#####\n"
		"#.+.#\n"
		"#####\n");
	RoguelikeGame::LevelData level = RoguelikeGame::LevelLoader::Parse(input, "doors");

	ASSERT_EQ(level.doors.size(), 1u);
	EXPECT_EQ(level.doors[0].doorId, "door_exit");
	EXPECT_EQ(level.doors[0].column, 2);
	EXPECT_EQ(level.doors[0].row, 1);
	EXPECT_EQ(level.tiles[1][2], RoguelikeGame::TileType::Door);
}

TEST(LevelLoaderTests, DoorWithoutAnIdIsRefused)
{
	std::istringstream input(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"+ Door:\n"
		"[map]\n"
		"#####\n"
		"#.+.#\n"
		"#####\n");

	EXPECT_THROW(RoguelikeGame::LevelLoader::Parse(input, "doors"), std::runtime_error);
}


// Забытый символ проваливался в Gap, который даже обзор не закрывает.
TEST(LevelLegendTest, AnOwnLegendAddsToTheDefaultOne)
{
	LevelData level = ParseLevel(
		"[legend]\n"
		"+ Door:door_test\n"
		"[map]\n"
		"###\n"
		"#+#\n"
		"###\n");

	EXPECT_EQ(CountTiles(level, TileType::Wall), 8) << "стандартный символ затёрт";
	EXPECT_EQ(CountTiles(level, TileType::Door), 1) << "своя строка легенды не сработала";
}

TEST(LevelLegendTest, AnOwnLineOverridesTheDefaultMeaning)
{
	LevelData level = ParseLevel(
		"[legend]\n"
		"# Floor\n"
		"[map]\n"
		"##\n");

	EXPECT_EQ(CountTiles(level, TileType::Floor), 2) << "объявленный символ не перекрыл стандартный";
	EXPECT_EQ(CountTiles(level, TileType::Wall), 0);
}
