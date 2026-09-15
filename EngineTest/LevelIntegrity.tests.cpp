#include "pch.h"
#include "ActAssembler.h"
#include "ItemCatalogLoader.h"
#include "LevelCatalog.h"
#include "LevelIntegrity.h"
#include "LevelLoader.h"
#include "ProjectFiles.h"
#include <sstream>

using RoguelikeGame::CheckLevel;
using RoguelikeGame::ItemCatalog;
using RoguelikeGame::ItemCatalogLoader;
using RoguelikeGame::LevelCatalog;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelEntry;
using RoguelikeGame::LevelFault;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::LevelReport;
using RoguelikeGame::LoadAct;

namespace
{
	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "integrity");
	}

	ItemCatalog KeyCatalog()
	{
		std::string text =
			"[item key_cell]\n"
			"name Cell key\n"
			"type Key\n"
			"icon Resources/Textures/items.png 0 0 16 16\n"
			"effect Unlock 0 door_cell\n"
			"\n"
			"[item key_gate]\n"
			"name Gate key\n"
			"type Key\n"
			"icon Resources/Textures/items.png 0 0 16 16\n"
			"effect Unlock 0 door_gate\n";
		std::istringstream input(text);

		return ItemCatalogLoader::Parse(input, "integrity");
	}

	const std::string DOOR_LEGEND =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"k Item:key_cell\n"
		"j Item:key_gate\n"
		"+ Door:door_cell\n"
		"= Door:door_gate\n"
		"[map]\n";
}

TEST(LevelIntegrityTest, AWalledRoomWithAStartIsClean)
{
	LevelData level = LevelOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"> Exit\n"
		"[map]\n"
		"#######\n"
		"#@...>#\n"
		"#######\n");

	EXPECT_TRUE(CheckLevel(level).IsClean()) << CheckLevel(level).Describe();
}

TEST(LevelIntegrityTest, AHoleInTheBorderIsReported)
{
	LevelData level = LevelOf(
		"#######\n"
		"#@....#\n"
		"###.###\n");

	LevelReport report = CheckLevel(level);

	EXPECT_EQ(report.Count(LevelFault::OpenBorder), 1);
}

TEST(LevelIntegrityTest, ASealedRoomIsUnreachable)
{
	LevelData level = LevelOf(
		"#########\n"
		"#@...#..#\n"
		"#########\n");

	LevelReport report = CheckLevel(level);

	EXPECT_EQ(report.Count(LevelFault::Unreachable), 2);
}

TEST(LevelIntegrityTest, AMapWithoutAStartIsReported)
{
	LevelData level = LevelOf(
		"#####\n"
		"#...#\n"
		"#####\n");

	LevelReport report = CheckLevel(level);

	EXPECT_EQ(report.Count(LevelFault::NoStart), 1);
}

TEST(LevelIntegrityTest, TwoStartsAreReported)
{
	LevelData level = LevelOf(
		"#######\n"
		"#@...@#\n"
		"#######\n");

	LevelReport report = CheckLevel(level);

	EXPECT_EQ(report.Count(LevelFault::ManyStarts), 1);
}

TEST(LevelIntegrityTest, AnEntranceNextToASpawnIsNotASecondStart)
{
	LevelData level = LevelOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"< Entrance\n"
		"[map]\n"
		"#######\n"
		"#@...<#\n"
		"#######\n");

	LevelReport report = CheckLevel(level);

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}

TEST(LevelIntegrityTest, ALockedDoorWithoutAKeyIsReported)
{
	LevelData level = LevelOf(DOOR_LEGEND +
		"#######\n"
		"#@.+..#\n"
		"#######\n");

	LevelReport report = CheckLevel(level, KeyCatalog());

	EXPECT_EQ(report.Count(LevelFault::LockedOut), 1);
	EXPECT_EQ(report.Count(LevelFault::Unreachable), 2);
}

TEST(LevelIntegrityTest, AKeyBeforeTheDoorOpensTheWay)
{
	LevelData level = LevelOf(DOOR_LEGEND +
		"########\n"
		"#@k+...#\n"
		"########\n");

	LevelReport report = CheckLevel(level, KeyCatalog());

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}

TEST(LevelIntegrityTest, AKeyBehindItsOwnDoorIsReported)
{
	LevelData level = LevelOf(DOOR_LEGEND +
		"########\n"
		"#@.+k..#\n"
		"########\n");

	LevelReport report = CheckLevel(level, KeyCatalog());

	EXPECT_EQ(report.Count(LevelFault::LockedOut), 1);
}

TEST(LevelIntegrityTest, DoorsOpenOneAfterAnother)
{
	LevelData level = LevelOf(DOOR_LEGEND +
		"###########\n"
		"#@k+j=....#\n"
		"###########\n");

	LevelReport report = CheckLevel(level, KeyCatalog());

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}

TEST(LevelIntegrityTest, AnUnreachableExitIsReported)
{
	LevelData level = LevelOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"> Exit\n"
		"[map]\n"
		"#########\n"
		"#@...#.>#\n"
		"#########\n");

	LevelReport report = CheckLevel(level);

	EXPECT_EQ(report.Count(LevelFault::ExitUnreachable), 1);
}

TEST(LevelIntegrityTest, ACrateDoesNotCloseTheWay)
{
	LevelData level = LevelOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"c Prop:crate_ammo\n"
		"> Exit\n"
		"[map]\n"
		"#######\n"
		"#@c..>#\n"
		"#######\n");

	LevelReport report = CheckLevel(level);

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}

namespace
{
	class ShippedLevelsTest : public ProjectFiles::Test
	{
	};
}

TEST_F(ShippedLevelsTest, EveryLevelInTheCatalogIsSound)
{
	ASSERT_TRUE(isFound) << "Resources/Levels/levels.config not found from " << previous.string();

	ItemCatalog items = ItemCatalogLoader::Load("Resources/Items/items.config");
	LevelCatalog catalog = LevelCatalog::Load("Resources/Levels/levels.config");
	ASSERT_FALSE(catalog.IsEmpty());

	for (const LevelEntry& entry : catalog)
	{
		LevelData level = entry.isAct ? LoadAct(entry.filePath) : LevelLoader::Load(entry.filePath);
		LevelReport report = CheckLevel(level, items);

		EXPECT_TRUE(report.IsClean()) << entry.id << "\n" << report.Describe();
	}
}
