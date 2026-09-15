#include "pch.h"
#include "LevelLoader.h"
#include "LevelZones.h"
#include <algorithm>
#include <sstream>

using RoguelikeGame::AreNeighbours;
using RoguelikeGame::BuildZones;
using RoguelikeGame::FindZoneAt;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::LevelZone;
using RoguelikeGame::ZonesWithin;

namespace
{
	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "zones");
	}

	const std::string THREE_IN_A_ROW =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"a Zone:left\n"
		"b Zone:middle\n"
		"c Zone:right\n"
		"[map]\n"
		"###############\n"
		"#a...b....c...#\n"
		"#.............#\n"
		"#...a....b...c#\n"
		"###############\n";

	const std::string TWO_ROOMS =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"a Zone:hall\n"
		"b Zone:vault\n"
		"[map]\n"
		"#########\n"
		"#a......#\n"
		"#...a...#\n"
		"####b...#\n"
		"#......b#\n"
		"#########\n";
}

TEST(LevelZonesTest, ZoneCoversTheMarkedCorners)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(TWO_ROOMS));

	ASSERT_EQ(zones.size(), 2u);
	EXPECT_EQ(zones[0].id, "hall");
	EXPECT_EQ(zones[0].minColumn, 1);
	EXPECT_EQ(zones[0].minRow, 1);
	EXPECT_EQ(zones[0].maxColumn, 4);
	EXPECT_EQ(zones[0].maxRow, 2);
}

TEST(LevelZonesTest, EachZoneKeepsItsOwnCells)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(TWO_ROOMS));

	ASSERT_EQ(zones.size(), 2u);
	EXPECT_EQ(zones[1].id, "vault");
	EXPECT_EQ(zones[1].minColumn, 4);
	EXPECT_EQ(zones[1].minRow, 3);
	EXPECT_EQ(zones[1].maxColumn, 7);
	EXPECT_EQ(zones[1].maxRow, 4);
}

TEST(LevelZonesTest, ZoneMarkerLeavesTheFloorWalkable)
{
	LevelData level = LevelOf(TWO_ROOMS);

	EXPECT_EQ(level.tiles[1][1], RoguelikeGame::TileType::Floor);
	EXPECT_EQ(level.zones.size(), 4u);
}

TEST(LevelZonesTest, CellInsideIsFoundAndOutsideIsNot)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(TWO_ROOMS));

	const LevelZone* hall = FindZoneAt(zones, 2, 1);
	ASSERT_NE(hall, nullptr);
	EXPECT_EQ(hall->id, "hall");

	EXPECT_EQ(FindZoneAt(zones, 7, 1), nullptr);
}

TEST(LevelZonesTest, CornerCellsBelongToTheZone)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(TWO_ROOMS));

	ASSERT_EQ(zones.size(), 2u);
	EXPECT_TRUE(zones[0].Contains(1, 1));
	EXPECT_TRUE(zones[0].Contains(4, 2));
	EXPECT_FALSE(zones[0].Contains(5, 2));
	EXPECT_FALSE(zones[0].Contains(4, 3));
}

TEST(LevelZonesTest, LevelWithoutZonesGivesNone)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf("[map]\n###\n#.#\n###\n"));

	EXPECT_TRUE(zones.empty());
	EXPECT_EQ(FindZoneAt(zones, 1, 1), nullptr);
}

TEST(LevelZonesTest, SingleMarkerMakesAZoneOfOneCell)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"a Zone:hall\n"
		"[map]\n"
		"#####\n"
		"#.a.#\n"
		"#####\n"));

	ASSERT_EQ(zones.size(), 1u);
	EXPECT_TRUE(zones[0].Contains(2, 1));
	EXPECT_FALSE(zones[0].Contains(1, 1));
}

TEST(LevelZonesTest, LegendWithoutAZoneIdIsRefused)
{
	EXPECT_THROW(LevelOf("[legend]\n# Wall\n. Floor\na Zone:\n[map]\n###\n#a#\n###\n"), std::runtime_error);
}

TEST(LevelZonesTest, RoomsSideBySideAreNeighbours)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(THREE_IN_A_ROW));

	ASSERT_EQ(zones.size(), 3u);
	EXPECT_TRUE(AreNeighbours(zones[0], zones[1], 1));
	EXPECT_TRUE(AreNeighbours(zones[1], zones[2], 1));
	EXPECT_FALSE(AreNeighbours(zones[0], zones[2], 1));
}

TEST(LevelZonesTest, ZoneIsNotItsOwnNeighbour)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(THREE_IN_A_ROW));

	ASSERT_FALSE(zones.empty());
	EXPECT_FALSE(AreNeighbours(zones[0], zones[0], 1));
}

TEST(LevelZonesTest, WithoutAGapTouchingRoomsAreApart)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(THREE_IN_A_ROW));

	ASSERT_EQ(zones.size(), 3u);
	EXPECT_FALSE(AreNeighbours(zones[0], zones[1], 0));
}

TEST(LevelZonesTest, LookingNowhereAheadGivesOnlyTheOwnRoom)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(THREE_IN_A_ROW));

	EXPECT_EQ(ZonesWithin(zones, 2, 1, 0, 1), std::vector<std::string>({"left"}));
}

TEST(LevelZonesTest, OneRoomAheadTakesTheNeighbour)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(THREE_IN_A_ROW));
	std::vector<std::string> woken = ZonesWithin(zones, 2, 1, 1, 1);

	ASSERT_EQ(woken.size(), 2u);
	EXPECT_EQ(woken[0], "left");
	EXPECT_EQ(woken[1], "middle");
}

TEST(LevelZonesTest, FarRoomStaysOutOfReach)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(THREE_IN_A_ROW));
	std::vector<std::string> woken = ZonesWithin(zones, 2, 1, 1, 1);

	EXPECT_EQ(std::count(woken.begin(), woken.end(), std::string("right")), 0);
}

TEST(LevelZonesTest, TwoRoomsAheadReachTheFarOne)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(THREE_IN_A_ROW));
	std::vector<std::string> woken = ZonesWithin(zones, 2, 1, 2, 1);

	EXPECT_EQ(woken.size(), 3u);
}

TEST(LevelZonesTest, StandingInTheMiddleReachesBothSides)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(THREE_IN_A_ROW));
	std::vector<std::string> woken = ZonesWithin(zones, 7, 1, 1, 1);

	ASSERT_EQ(woken.size(), 3u);
	EXPECT_EQ(woken[0], "middle");
}

TEST(LevelZonesTest, OutsideEveryRoomWakesNothing)
{
	std::vector<LevelZone> zones = BuildZones(LevelOf(THREE_IN_A_ROW));

	EXPECT_TRUE(ZonesWithin(zones, 0, 0, 1, 1).empty());
}
