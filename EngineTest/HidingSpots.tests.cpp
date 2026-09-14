#include "pch.h"
#include "GameSettings.h"
#include "HidingSpots.h"
#include "LevelLoader.h"
#include <sstream>

using RoguelikeGame::FindHidingSpots;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PathField;
using XYZEngine::Vector2Df;

namespace
{
	LevelGrid GridOf(const std::string& map)
	{
		std::istringstream input(map);

		return LevelGrid::Build(LevelLoader::Parse(input, "spots"));
	}

	const std::string NOOKS =
		"[map]\n"
		"#########\n"
		"#.......#\n"
		"#.#####.#\n"
		"#.......#\n"
		"#.#####.#\n"
		"#.......#\n"
		"#########\n";

	const std::string OPEN_HALL =
		"[map]\n"
		"#######\n"
		"#.....#\n"
		"#.....#\n"
		"#.....#\n"
		"#######\n";
}

TEST(HidingSpotsTest, SpotsAreOutOfSightFromThePoint)
{
	LevelGrid grid = GridOf(NOOKS);
	PathField field;
	field.Build(grid, 4, 1);

	std::vector<Vector2Df> spots = FindHidingSpots(grid, field, grid.ToWorld(4, 1), 5, 4u);

	ASSERT_FALSE(spots.empty());
	for (const Vector2Df& spot : spots)
	{
		EXPECT_TRUE(grid.HasWallBetween(grid.ToWorld(4, 1), spot));
	}
}

TEST(HidingSpotsTest, SpotsAreWalkable)
{
	LevelGrid grid = GridOf(NOOKS);
	PathField field;
	field.Build(grid, 4, 1);

	std::vector<Vector2Df> spots = FindHidingSpots(grid, field, grid.ToWorld(4, 1), 5, 4u);

	ASSERT_FALSE(spots.empty());
	for (const Vector2Df& spot : spots)
	{
		int column = 0;
		int row = 0;
		grid.ToCell(spot, column, row);

		EXPECT_TRUE(grid.IsPassable(column, row));
		EXPECT_TRUE(field.IsReachable(column, row));
	}
}

TEST(HidingSpotsTest, NearestSpotsComeFirst)
{
	LevelGrid grid = GridOf(NOOKS);
	PathField field;
	field.Build(grid, 4, 1);

	std::vector<Vector2Df> spots = FindHidingSpots(grid, field, grid.ToWorld(4, 1), 5, 4u);
	ASSERT_GT(spots.size(), 1u);

	int previous = -1;
	for (const Vector2Df& spot : spots)
	{
		int column = 0;
		int row = 0;
		grid.ToCell(spot, column, row);

		EXPECT_GE(field.GetDistance(column, row), previous);
		previous = field.GetDistance(column, row);
	}
}

TEST(HidingSpotsTest, NoMoreSpotsThanAsked)
{
	LevelGrid grid = GridOf(NOOKS);
	PathField field;
	field.Build(grid, 4, 1);

	EXPECT_LE(FindHidingSpots(grid, field, grid.ToWorld(4, 1), 5, 1u).size(), 1u);
	EXPECT_LE(FindHidingSpots(grid, field, grid.ToWorld(4, 1), 5, 2u).size(), 2u);
}

TEST(HidingSpotsTest, OpenRoomHasNowhereToHide)
{
	LevelGrid grid = GridOf(OPEN_HALL);
	PathField field;
	field.Build(grid, 3, 1);

	EXPECT_TRUE(FindHidingSpots(grid, field, grid.ToWorld(3, 1), 5, 4u).empty());
}

TEST(HidingSpotsTest, SmallRadiusFindsNothingFarAway)
{
	LevelGrid grid = GridOf(NOOKS);
	PathField field;
	field.Build(grid, 4, 1);

	EXPECT_TRUE(FindHidingSpots(grid, field, grid.ToWorld(4, 1), 0, 4u).empty());
}

TEST(HidingSpotsTest, AskingForNothingGivesNothing)
{
	LevelGrid grid = GridOf(NOOKS);
	PathField field;
	field.Build(grid, 4, 1);

	EXPECT_TRUE(FindHidingSpots(grid, field, grid.ToWorld(4, 1), 5, 0u).empty());
}

TEST(HidingSpotsTest, WithoutALevelThereAreNoSpots)
{
	LevelGrid grid;
	PathField field;

	EXPECT_TRUE(FindHidingSpots(grid, field, {0.f, 0.f}, 5, 4u).empty());
}
