#include "pch.h"
#include "BackOffSpot.h"
#include "GameSettings.h"
#include "LevelLoader.h"
#include <sstream>

using RoguelikeGame::FindBackOffSpot;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PathField;
using XYZEngine::Vector2Df;

namespace
{
	constexpr float WANTED = 4.f * RoguelikeGame::TILE_SIZE;
	constexpr int RADIUS = 5;

	// Короче тайла: все дальние клетки равны по выигрышу, и решает одна лишь цена хода.
	constexpr float CLOSE_QUARTERS = 40.f;

	LevelGrid GridOf(const std::string& map)
	{
		std::istringstream input(map);

		return LevelGrid::Build(LevelLoader::Parse(input, "backoff"));
	}

	const std::string HALL =
		"[map]\n"
		"###########\n"
		"#.........#\n"
		"#.........#\n"
		"#.........#\n"
		"###########\n";

	// Комната справа отрезана наглухо: она дальше от игрока, но дойти до неё нельзя.
	const std::string SEALED =
		"[map]\n"
		"###########\n"
		"#....#....#\n"
		"#....#....#\n"
		"#....######\n"
		"###########\n";

	// Тупик: врагу дальше отходить некуда вовсе.
	const std::string DEAD_END =
		"[map]\n"
		"#####\n"
		"#...#\n"
		"#####\n";

	struct Spot
	{
		bool found = false;
		Vector2Df place;
		int column = 0;
		int row = 0;
	};

	Spot Ask(const LevelGrid& grid, int fromColumn, int fromRow, int threatColumn, int threatRow)
	{
		PathField field;
		field.Build(grid, fromColumn, fromRow);

		Spot spot;
		spot.found = FindBackOffSpot(grid, field, grid.ToWorld(fromColumn, fromRow),
			grid.ToWorld(threatColumn, threatRow), WANTED, RADIUS, spot.place);

		if (spot.found)
		{
			grid.ToCell(spot.place, spot.column, spot.row);
		}

		return spot;
	}
}

TEST(BackOffSpotTest, TheSpotIsFurtherFromTheThreatThanTheEnemyIs)
{
	LevelGrid grid = GridOf(HALL);
	Vector2Df threat = grid.ToWorld(2, 2);
	Vector2Df from = grid.ToWorld(4, 2);

	Spot spot = Ask(grid, 4, 2, 2, 2);

	ASSERT_TRUE(spot.found);
	EXPECT_GT((spot.place - threat).GetLength(), (from - threat).GetLength());
}

TEST(BackOffSpotTest, TheSpotIsWalkable)
{
	LevelGrid grid = GridOf(HALL);

	Spot spot = Ask(grid, 4, 2, 2, 2);

	ASSERT_TRUE(spot.found);
	EXPECT_TRUE(grid.IsPassable(spot.column, spot.row));
}

TEST(BackOffSpotTest, AWalledOffRoomIsNotChosenHoweverFarItIs)
{
	LevelGrid grid = GridOf(SEALED);

	Spot spot = Ask(grid, 4, 2, 1, 2);

	ASSERT_TRUE(spot.found) << "there was still room on this side of the wall";
	EXPECT_LE(spot.column, 4) << "the enemy is backing off into a room it cannot walk to";
}

TEST(BackOffSpotTest, ACorneredEnemyHasNowhereToGo)
{
	LevelGrid grid = GridOf(DEAD_END);

	Spot spot = Ask(grid, 3, 1, 1, 1);

	EXPECT_FALSE(spot.found) << "found room in a one-tile dead end";
}

TEST(BackOffSpotTest, TheAnswerIsTheSameEveryTime)
{
	LevelGrid grid = GridOf(HALL);

	Spot first = Ask(grid, 4, 2, 2, 2);
	Spot again = Ask(grid, 4, 2, 2, 2);

	ASSERT_TRUE(first.found);
	ASSERT_TRUE(again.found);
	EXPECT_EQ(first.column, again.column);
	EXPECT_EQ(first.row, again.row);
}

TEST(BackOffSpotTest, AnEmptyGridIsNotSearched)
{
	LevelGrid grid;
	PathField field;
	Vector2Df spot;

	EXPECT_FALSE(FindBackOffSpot(grid, field, {0.f, 0.f}, {100.f, 0.f}, WANTED, RADIUS, spot));
}

TEST(BackOffSpotTest, TheEnemyDoesNotRetreatThroughTheThreat)
{
	LevelGrid grid = GridOf(HALL);
	Vector2Df threat = grid.ToWorld(4, 2);
	Vector2Df from = threat + Vector2Df{20.f, 0.f};

	PathField field;
	field.Build(grid, 4, 2);

	Vector2Df place;
	ASSERT_TRUE(FindBackOffSpot(grid, field, from, threat, CLOSE_QUARTERS, RADIUS, place));

	EXPECT_GT(place.x, threat.x) << "the enemy backs off by walking straight through the hero";
}
