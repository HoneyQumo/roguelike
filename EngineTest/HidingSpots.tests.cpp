#include "pch.h"
#include "GameSettings.h"
#include "HidingSpots.h"
#include "LevelLoader.h"
#include <sstream>
#include <algorithm>
#include <cstdlib>

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

	std::vector<Vector2Df> spots = FindHidingSpots(grid, field, grid.ToWorld(4, 1), {0.f, 0.f}, 5, 4u, 1);

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

	std::vector<Vector2Df> spots = FindHidingSpots(grid, field, grid.ToWorld(4, 1), {0.f, 0.f}, 5, 4u, 1);

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

	std::vector<Vector2Df> spots = FindHidingSpots(grid, field, grid.ToWorld(4, 1), {0.f, 0.f}, 5, 4u, 1);
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

	EXPECT_LE(FindHidingSpots(grid, field, grid.ToWorld(4, 1), {0.f, 0.f}, 5, 1u, 1).size(), 1u);
	EXPECT_LE(FindHidingSpots(grid, field, grid.ToWorld(4, 1), {0.f, 0.f}, 5, 2u, 1).size(), 2u);
}

TEST(HidingSpotsTest, OpenRoomHasNowhereToHide)
{
	LevelGrid grid = GridOf(OPEN_HALL);
	PathField field;
	field.Build(grid, 3, 1);

	EXPECT_TRUE(FindHidingSpots(grid, field, grid.ToWorld(3, 1), {0.f, 0.f}, 5, 4u, 1).empty());
}

TEST(HidingSpotsTest, SmallRadiusFindsNothingFarAway)
{
	LevelGrid grid = GridOf(NOOKS);
	PathField field;
	field.Build(grid, 4, 1);

	EXPECT_TRUE(FindHidingSpots(grid, field, grid.ToWorld(4, 1), {0.f, 0.f}, 0, 4u, 1).empty());
}

TEST(HidingSpotsTest, AskingForNothingGivesNothing)
{
	LevelGrid grid = GridOf(NOOKS);
	PathField field;
	field.Build(grid, 4, 1);

	EXPECT_TRUE(FindHidingSpots(grid, field, grid.ToWorld(4, 1), {0.f, 0.f}, 5, 0u, 1).empty());
}

TEST(HidingSpotsTest, WithoutALevelThereAreNoSpots)
{
	LevelGrid grid;
	PathField field;

	EXPECT_TRUE(FindHidingSpots(grid, field, {0.f, 0.f}, {0.f, 0.f}, 5, 4u, 1).empty());
}

TEST(HidingSpotsTest, SpotsAreSpreadApart)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"##########\n"
		"#........#\n"
		"#.######.#\n"
		"#........#\n"
		"#.######.#\n"
		"#........#\n"
		"#.######.#\n"
		"#........#\n"
		"##########\n");

	PathField field;
	field.Build(grid, 4, 1);

	std::vector<Vector2Df> spots = FindHidingSpots(grid, field, grid.ToWorld(4, 1), {0.f, 0.f}, 8, 3u, 3);
	ASSERT_GT(spots.size(), 1u);

	for (std::size_t first = 0u; first < spots.size(); first++)
	{
		for (std::size_t second = first + 1u; second < spots.size(); second++)
		{
			int firstColumn = 0;
			int firstRow = 0;
			int secondColumn = 0;
			int secondRow = 0;
			grid.ToCell(spots[first], firstColumn, firstRow);
			grid.ToCell(spots[second], secondColumn, secondRow);

			int gap = std::max(std::abs(firstColumn - secondColumn), std::abs(firstRow - secondRow));
			EXPECT_GE(gap, 3);
		}
	}
}

TEST(HidingSpotsTest, TheLastPlaceIsAtTheFarEnd)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"##########\n"
		"#........#\n"
		"#.######.#\n"
		"#........#\n"
		"#.######.#\n"
		"#........#\n"
		"#.######.#\n"
		"#........#\n"
		"##########\n");

	PathField field;
	field.Build(grid, 4, 1);

	std::vector<Vector2Df> spots = FindHidingSpots(grid, field, grid.ToWorld(4, 1), {0.f, 0.f}, 8, 2u, 3);
	ASSERT_EQ(spots.size(), 2u);

	int column = 0;
	int row = 0;
	grid.ToCell(spots.back(), column, row);
	int furthest = field.GetDistance(column, row);

	grid.ToCell(spots.front(), column, row);
	int nearest = field.GetDistance(column, row);

	EXPECT_GT(furthest, nearest);
}

TEST(HidingSpotsTest, SpotIsRightBehindACorner)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"#######\n"
		"#.....#\n"
		"#.###.#\n"
		"#.....#\n"
		"#######\n");

	Vector2Df from = grid.ToWorld(1, 1);

	EXPECT_TRUE(RoguelikeGame::IsJustBehindACorner(grid, from, 5, 2));
	EXPECT_FALSE(RoguelikeGame::IsJustBehindACorner(grid, from, 2, 1));
	EXPECT_FALSE(RoguelikeGame::IsJustBehindACorner(grid, from, 2, 2));
	EXPECT_FALSE(RoguelikeGame::IsJustBehindACorner(grid, from, 4, 3));
}

TEST(HidingSpotsTest, TwoCornersGiveTwoPlaces)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"#########\n"
		"#.......#\n"
		"#.#####.#\n"
		"#.......#\n"
		"#.#####.#\n"
		"#.......#\n"
		"#########\n");

	PathField field;
	field.Build(grid, 4, 1);

	std::vector<Vector2Df> spots = FindHidingSpots(grid, field, grid.ToWorld(4, 1), {0.f, 0.f}, 6, 2u, 1);

	EXPECT_EQ(spots.size(), 2u);
}

TEST(HidingSpotsTest, SearchFollowsTheWayTheTargetRan)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"###########\n"
		"#.........#\n"
		"#.#######.#\n"
		"#.........#\n"
		"###########\n");

	PathField field;
	field.Build(grid, 5, 1);
	Vector2Df from = grid.ToWorld(5, 1);

	std::vector<Vector2Df> toTheRight = FindHidingSpots(grid, field, from, {1.f, 0.f}, 8, 3u, 1);
	ASSERT_FALSE(toTheRight.empty());

	for (const Vector2Df& spot : toTheRight)
	{
		EXPECT_GT(spot.x, from.x) << "spot at " << spot.x << ";" << spot.y;
	}

	std::vector<Vector2Df> toTheLeft = FindHidingSpots(grid, field, from, {-1.f, 0.f}, 8, 3u, 1);
	ASSERT_FALSE(toTheLeft.empty());

	for (const Vector2Df& spot : toTheLeft)
	{
		EXPECT_LT(spot.x, from.x) << "spot at " << spot.x << ";" << spot.y;
	}
}

TEST(HidingSpotsTest, WithoutADirectionBothSidesAreChecked)
{
	LevelGrid grid = GridOf(
		"[map]\n"
		"###########\n"
		"#.........#\n"
		"#.#######.#\n"
		"#.........#\n"
		"###########\n");

	PathField field;
	field.Build(grid, 5, 1);
	Vector2Df from = grid.ToWorld(5, 1);

	std::vector<Vector2Df> spots = FindHidingSpots(grid, field, from, {0.f, 0.f}, 8, 6u, 1);
	ASSERT_GT(spots.size(), 1u);

	bool hasLeft = false;
	bool hasRight = false;
	for (const Vector2Df& spot : spots)
	{
		hasLeft = hasLeft || spot.x < from.x;
		hasRight = hasRight || spot.x > from.x;
	}

	EXPECT_TRUE(hasLeft);
	EXPECT_TRUE(hasRight);
}
