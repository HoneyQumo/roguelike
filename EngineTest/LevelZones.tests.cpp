#include "pch.h"
#include "LevelLoader.h"
#include "LevelZones.h"
#include <sstream>

using RoguelikeGame::BuildZones;
using RoguelikeGame::FindZoneAt;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::LevelZone;

namespace
{
	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "zones");
	}

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
