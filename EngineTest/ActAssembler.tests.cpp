#include "pch.h"
#include "ActAssembler.h"
#include "LevelLoader.h"
#include "LevelZones.h"
#include <map>
#include <sstream>

using RoguelikeGame::ActPlan;
using RoguelikeGame::AssembleAct;
using RoguelikeGame::BuildZones;
using RoguelikeGame::LevelZone;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::RoomPlacement;
using RoguelikeGame::TileType;

namespace
{
	LevelData RoomOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "room");
	}

	const std::string LEFT =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"1 Watch:guard\n"
		"[map]\n"
		"#####\n"
		"#...#\n"
		"#...1\n"
		"#...#\n"
		"#####\n";

	const std::string RIGHT =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"p Item:potion_small\n"
		"[map]\n"
		"#####\n"
		"#..p#\n"
		"....#\n"
		"#...#\n"
		"#####\n";

	const std::string ZONED =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"z Zone:inner\n"
		"[map]\n"
		"#####\n"
		"#z..#\n"
		"#...#\n"
		"#.z.#\n"
		"#####\n";

	const std::string CLOSED =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"[map]\n"
		"#####\n"
		"#...#\n"
		"#...#\n"
		"#...#\n"
		"#####\n";

	class Library
	{
	public:
		void Add(const std::string& id, const LevelData& room) { rooms[id] = room; }

		RoguelikeGame::RoomSource Source() const
		{
			return [this](const std::string& id) -> const LevelData*
			{
				auto found = rooms.find(id);

				return found == rooms.end() ? nullptr : &found->second;
			};
		}

	private:
		std::map<std::string, LevelData> rooms;
	};

	ActPlan PlanOf(std::vector<RoomPlacement> rooms)
	{
		ActPlan plan;
		plan.info.title = "Акт";
		plan.rooms = std::move(rooms);

		return plan;
	}
}

TEST(ActAssemblerTest, TwoRoomsBecomeOneMap)
{
	Library library;
	library.Add("left", RoomOf(LEFT));
	library.Add("right", RoomOf(RIGHT));

	LevelData act = AssembleAct(PlanOf({{"left", 0, 0, 0, false}, {"right", 5, 0, 0, false}}), library.Source());

	EXPECT_EQ(act.width, 10);
	EXPECT_EQ(act.height, 5);
	EXPECT_EQ(act.info.title, "Акт");
}

TEST(ActAssemblerTest, RoomTilesLandWhereTheyWerePut)
{
	Library library;
	library.Add("left", RoomOf(LEFT));
	library.Add("right", RoomOf(RIGHT));

	LevelData act = AssembleAct(PlanOf({{"left", 0, 0, 0, false}, {"right", 5, 0, 0, false}}), library.Source());

	EXPECT_EQ(act.tiles[0][0], TileType::Wall);
	EXPECT_EQ(act.tiles[2][4], TileType::Floor);
	EXPECT_EQ(act.tiles[2][5], TileType::Floor);
}

TEST(ActAssemblerTest, ThingsMoveWithTheirRoom)
{
	Library library;
	library.Add("left", RoomOf(LEFT));
	library.Add("right", RoomOf(RIGHT));

	LevelData act = AssembleAct(PlanOf({{"left", 0, 0, 0, false}, {"right", 5, 0, 0, false}}), library.Source());

	ASSERT_EQ(act.items.size(), 1u);
	EXPECT_EQ(act.items[0].column, 8);
	EXPECT_EQ(act.items[0].row, 1);
}

TEST(ActAssemblerTest, PatrolRoutesOfDifferentRoomsDoNotMerge)
{
	Library library;
	library.Add("left", RoomOf(LEFT));

	LevelData act = AssembleAct(PlanOf({{"left", 0, 0, 0, false}, {"left", 0, 5, 0, false}}), library.Source());

	ASSERT_EQ(act.patrols.size(), 2u);
	EXPECT_NE(act.patrols[0].routeId, "");
	EXPECT_EQ(act.patrols[0].routeId, act.patrols[1].routeId);
}

TEST(ActAssemblerTest, TurnedRoomKeepsItsThingsOnTiles)
{
	Library library;
	library.Add("right", RoomOf(RIGHT));

	LevelData act = AssembleAct(PlanOf({{"right", 0, 0, 1, false}}), library.Source());

	ASSERT_EQ(act.items.size(), 1u);
	EXPECT_EQ(act.tiles[act.items[0].row][act.items[0].column], TileType::Floor);
}

TEST(ActAssemblerTest, OpeningIntoAWallIsRefused)
{
	Library library;
	library.Add("left", RoomOf(LEFT));
	library.Add("closed", RoomOf(CLOSED));

	EXPECT_THROW(AssembleAct(PlanOf({{"left", 0, 0, 0, false}, {"closed", 5, 0, 0, false}}), library.Source()),
		std::runtime_error);
}

TEST(ActAssemblerTest, RoomsThatDoNotTouchAreNotChecked)
{
	Library library;
	library.Add("left", RoomOf(LEFT));
	library.Add("closed", RoomOf(CLOSED));

	EXPECT_NO_THROW(AssembleAct(PlanOf({{"left", 0, 0, 0, false}, {"closed", 9, 0, 0, false}}), library.Source()));
}

TEST(ActAssemblerTest, UnknownRoomIsRefused)
{
	Library library;
	library.Add("left", RoomOf(LEFT));

	EXPECT_THROW(AssembleAct(PlanOf({{"nowhere", 0, 0, 0, false}}), library.Source()), std::runtime_error);
}

TEST(ActAssemblerTest, CellsOutsideEveryRoomStayEmpty)
{
	Library library;
	library.Add("left", RoomOf(LEFT));

	LevelData act = AssembleAct(PlanOf({{"left", 0, 0, 0, false}, {"left", 0, 6, 0, false}}), library.Source());

	EXPECT_EQ(act.tiles[5][0], TileType::Empty);
}

TEST(ActAssemblerTest, RoomWithoutZonesBecomesOneZone)
{
	Library library;
	library.Add("left", RoomOf(LEFT));
	library.Add("right", RoomOf(RIGHT));

	LevelData act = AssembleAct(PlanOf({{"left", 0, 0, 0, false}, {"right", 5, 0, 0, false}}), library.Source());
	std::vector<LevelZone> zones = BuildZones(act);

	ASSERT_EQ(zones.size(), 2u);
	EXPECT_TRUE(zones[0].Contains(0, 0));
	EXPECT_TRUE(zones[0].Contains(4, 4));
	EXPECT_FALSE(zones[0].Contains(5, 0));
	EXPECT_TRUE(zones[1].Contains(5, 0));
	EXPECT_TRUE(zones[1].Contains(9, 4));
}

TEST(ActAssemblerTest, SameRoomPlacedTwiceGetsTwoZones)
{
	Library library;
	library.Add("left", RoomOf(LEFT));

	LevelData act = AssembleAct(PlanOf({{"left", 0, 0, 0, false}, {"left", 0, 6, 0, false}}), library.Source());
	std::vector<LevelZone> zones = BuildZones(act);

	ASSERT_EQ(zones.size(), 2u);
	EXPECT_NE(zones[0].id, zones[1].id);
	EXPECT_FALSE(zones[0].Contains(0, 6));
	EXPECT_FALSE(zones[1].Contains(0, 0));
}

TEST(ActAssemblerTest, RoomKeepsItsOwnZonesInsteadOfTheWholeRoom)
{
	Library library;
	library.Add("zoned", RoomOf(ZONED));

	LevelData act = AssembleAct(PlanOf({{"zoned", 10, 0, 0, false}}), library.Source());
	std::vector<LevelZone> zones = BuildZones(act);

	ASSERT_EQ(zones.size(), 1u);
	EXPECT_EQ(zones[0].minColumn, 11);
	EXPECT_EQ(zones[0].minRow, 1);
	EXPECT_EQ(zones[0].maxColumn, 12);
	EXPECT_EQ(zones[0].maxRow, 3);
}

TEST(ActAssemblerTest, TurnedRoomCarriesItsZone)
{
	Library library;
	library.Add("zoned", RoomOf(ZONED));

	LevelData act = AssembleAct(PlanOf({{"zoned", 0, 0, 1, false}}), library.Source());
	std::vector<LevelZone> zones = BuildZones(act);

	ASSERT_EQ(zones.size(), 1u);
	EXPECT_EQ(zones[0].minColumn, 1);
	EXPECT_EQ(zones[0].minRow, 1);
	EXPECT_EQ(zones[0].maxColumn, 3);
	EXPECT_EQ(zones[0].maxRow, 2);
}
