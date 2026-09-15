#include "pch.h"
#include "ActAssembler.h"
#include "ItemCatalogLoader.h"
#include "PropCatalog.h"
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

	RoguelikeGame::PropCatalog PropKinds()
	{
		std::string text =
			"[prop bunk]\n"
			"name Bunk\n"
			"size 56\n"
			"\n"
			"[prop crate]\n"
			"name Crate\n"
			"health 40\n"
			"size 48\n"
			"\n"
			"[prop bush]\n"
			"name Bush\n"
			"solid false\n"
			"size 56\n";
		std::istringstream input(text);

		return RoguelikeGame::PropCatalog::Parse(input, "integrity");
	}

	const std::string PROP_LEGEND =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"n Prop:bunk\n"
		"c Prop:crate\n"
		"h Prop:bush\n"
		"[map]\n";

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
	RoguelikeGame::PropCatalog props = RoguelikeGame::PropCatalog::Load("Resources/Props/props.config");
	LevelCatalog catalog = LevelCatalog::Load("Resources/Levels/levels.config");
	ASSERT_FALSE(catalog.IsEmpty());

	for (const LevelEntry& entry : catalog)
	{
		LevelData level = entry.isAct ? LoadAct(entry.filePath) : LevelLoader::Load(entry.filePath);
		LevelReport report = CheckLevel(level, items, props);

		EXPECT_TRUE(report.IsClean()) << entry.id << "\n" << report.Describe();
	}
}

namespace
{
	class ShippedRoomsTest : public ProjectFiles::Test
	{
	};
}

TEST_F(ShippedRoomsTest, EveryRoomIsRectangular)
{
	ASSERT_TRUE(isFound) << previous.string();

	int rooms = 0;
	for (const auto& entry : std::filesystem::directory_iterator("Resources/Rooms"))
	{
		LevelData room = LevelLoader::Load(entry.path().string());
		rooms++;

		for (std::size_t line = 0; line < room.tiles.size(); line++)
		{
			EXPECT_EQ(static_cast<int>(room.tiles[line].size()), room.width)
				<< entry.path().filename().string() << " line " << line;
		}
	}

	EXPECT_GT(rooms, 0);
}

TEST_F(ShippedRoomsTest, EveryRoomNamesThingsThatExist)
{
	ASSERT_TRUE(isFound) << previous.string();

	ItemCatalog items = ItemCatalogLoader::Load("Resources/Items/items.config");
	RoguelikeGame::PropCatalog props = RoguelikeGame::PropCatalog::Load("Resources/Props/props.config");

	for (const auto& entry : std::filesystem::directory_iterator("Resources/Rooms"))
	{
		std::string name = entry.path().filename().string();
		LevelData room = LevelLoader::Load(entry.path().string());

		for (const RoguelikeGame::PropPlacement& prop : room.props)
		{
			EXPECT_NE(props.Find(prop.propId), nullptr) << name << " has " << prop.propId;
		}

		for (const RoguelikeGame::ItemPlacement& item : room.items)
		{
			EXPECT_TRUE(items.Contains(item.itemId)) << name << " has " << item.itemId;
		}
	}
}

TEST(LevelIntegrityTest, AnUnbreakableThingInTheOnlyPassageSealsTheWay)
{
	LevelData level = LevelOf(PROP_LEGEND +
		"#######\n"
		"#@.#..#\n"
		"#..n..#\n"
		"#..#..#\n"
		"#######\n");

	LevelReport report = CheckLevel(level, ItemCatalog(), PropKinds());

	EXPECT_GT(report.Count(LevelFault::Unreachable), 0) << report.Describe();
}

TEST(LevelIntegrityTest, ABreakableCrateInTheSamePassageIsFine)
{
	LevelData level = LevelOf(PROP_LEGEND +
		"#######\n"
		"#@.#..#\n"
		"#..c..#\n"
		"#..#..#\n"
		"#######\n");

	LevelReport report = CheckLevel(level, ItemCatalog(), PropKinds());

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}

TEST(LevelIntegrityTest, AThingYouWalkThroughDoesNotSealTheWay)
{
	LevelData level = LevelOf(PROP_LEGEND +
		"#######\n"
		"#@.#..#\n"
		"#..h..#\n"
		"#..#..#\n"
		"#######\n");

	LevelReport report = CheckLevel(level, ItemCatalog(), PropKinds());

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}
