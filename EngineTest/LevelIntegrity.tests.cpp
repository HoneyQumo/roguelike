#include "pch.h"
#include "ActAssembler.h"
#include "ItemCatalogLoader.h"
#include "PropCatalog.h"
#include "LevelCatalog.h"
#include "LevelIntegrity.h"
#include "LevelLoader.h"
#include "LevelProgression.h"
#include "ProjectFiles.h"
#include <set>
#include <sstream>

using RoguelikeGame::CheckLevel;
using RoguelikeGame::ItemCatalog;
using RoguelikeGame::ItemCatalogLoader;
using RoguelikeGame::LevelCatalog;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelEntry;
using RoguelikeGame::LevelMode;
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

// \u041a\u043e\u043b\u044c\u0446\u043e \u0432 \u0440\u0435\u0435\u0441\u0442\u0440\u0435 \u0434\u0435\u043b\u0430\u043b\u043e \u044d\u043a\u0440\u0430\u043d \u043f\u043e\u0431\u0435\u0434\u044b \u043d\u0435\u0434\u043e\u0441\u0442\u0438\u0436\u0438\u043c\u044b\u043c, \u0438 \u043d\u0438 \u043e\u0434\u0438\u043d \u0442\u0435\u0441\u0442 \u044d\u0442\u043e\u0433\u043e \u043d\u0435 \u0432\u0438\u0434\u0435\u043b:
// \u043f\u0440\u043e\u0432\u0435\u0440\u044f\u043b\u0438 \u043a\u0430\u0436\u0434\u0443\u044e \u043a\u0430\u0440\u0442\u0443 \u043f\u043e \u043e\u0442\u0434\u0435\u043b\u044c\u043d\u043e\u0441\u0442\u0438, \u0430 \u043d\u0435 \u043f\u0443\u0442\u044c \u0446\u0435\u043b\u0438\u043a\u043e\u043c.
TEST_F(ShippedLevelsTest, TheCampaignChainEndsAndNeverLoops)
{
	ASSERT_TRUE(isFound) << "Resources/Levels/levels.config not found from " << previous.string();

	LevelCatalog catalog = LevelCatalog::Load("Resources/Levels/levels.config");
	ASSERT_FALSE(catalog.IsEmpty());

	int index = catalog.FirstIndex(LevelMode::Campaign);
	ASSERT_GE(index, 0) << "\u0432 \u0440\u0435\u0435\u0441\u0442\u0440\u0435 \u043d\u0435\u0442 \u043d\u0438 \u043e\u0434\u043d\u043e\u0439 \u0441\u044e\u0436\u0435\u0442\u043d\u043e\u0439 \u043b\u043e\u043a\u0430\u0446\u0438\u0438";

	std::set<int> visited;
	int walked = 0;

	while (true)
	{
		const LevelEntry* entry = catalog.GetAt(index);
		ASSERT_NE(entry, nullptr);
		ASSERT_TRUE(visited.insert(index).second) << "\u0446\u0435\u043f\u043e\u0447\u043a\u0430 \u0432\u0435\u0440\u043d\u0443\u043b\u0430\u0441\u044c \u0432 " << entry->id;
		ASSERT_EQ(entry->mode, LevelMode::Campaign)
			<< entry->id << " \u043d\u0435 \u0441\u044e\u0436\u0435\u0442\u043d\u0430\u044f, \u0430 \u0441\u0442\u043e\u0438\u0442 \u0432 \u0446\u0435\u043f\u043e\u0447\u043a\u0435 \u043f\u0440\u043e\u0445\u043e\u0436\u0434\u0435\u043d\u0438\u044f";

		walked++;

		LevelData level = entry->isAct ? LoadAct(entry->filePath) : LevelLoader::Load(entry->filePath);
		RoguelikeGame::LevelStep step = RoguelikeGame::ResolveNextLevel(catalog, index, level.info.nextLevelId);

		ASSERT_NE(step.kind, RoguelikeGame::LevelStepKind::Unknown)
			<< entry->id << " \u0432\u0435\u0434\u0451\u0442 \u0432 \u043d\u0435\u0438\u0437\u0432\u0435\u0441\u0442\u043d\u0443\u044e \u043b\u043e\u043a\u0430\u0446\u0438\u044e " << level.info.nextLevelId;

		if (step.kind == RoguelikeGame::LevelStepKind::Finished)
		{
			break;
		}

		index = step.index;
	}

	EXPECT_GE(walked, 3) << "\u0432 \u043a\u0430\u043c\u043f\u0430\u043d\u0438\u0438 \u043c\u0435\u043d\u044c\u0448\u0435 \u0442\u0440\u0451\u0445 \u043b\u043e\u043a\u0430\u0446\u0438\u0439";
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
