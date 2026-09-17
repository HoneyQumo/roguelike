#include "pch.h"
#include "CoverSpot.h"
#include "GameSettings.h"
#include "LevelLoader.h"
#include <sstream>

using RoguelikeGame::FindCoverSpot;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PathField;
using XYZEngine::Vector2Df;

namespace
{
	constexpr int RADIUS = 6;

	LevelGrid GridOf(const std::string& map)
	{
		std::istringstream input(map);

		return LevelGrid::Build(LevelLoader::Parse(input, "cover"));
	}

	// Столб посреди зала: за ним можно встать, и оттуда игрока не видно.
	const std::string PILLAR =
		"[map]\n"
		"#########\n"
		"#.......#\n"
		"#...#...#\n"
		"#.......#\n"
		"#########\n";

	const std::string OPEN_HALL =
		"[map]\n"
		"#########\n"
		"#.......#\n"
		"#.......#\n"
		"#.......#\n"
		"#########\n";

	// Комната справа отрезана наглухо: укрытие идеальное, но дойти нельзя.
	const std::string SEALED =
		"[map]\n"
		"###########\n"
		"#....#....#\n"
		"#....#....#\n"
		"#....######\n"
		"###########\n";

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
		spot.found = FindCoverSpot(grid, field, grid.ToWorld(fromColumn, fromRow),
			grid.ToWorld(threatColumn, threatRow), RADIUS, spot.place);

		if (spot.found)
		{
			grid.ToCell(spot.place, spot.column, spot.row);
		}

		return spot;
	}
}

TEST(CoverSpotTest, TheSpotIsOutOfSightOfTheThreat)
{
	LevelGrid grid = GridOf(PILLAR);
	Vector2Df threat = grid.ToWorld(1, 2);

	Spot spot = Ask(grid, 3, 2, 1, 2);

	ASSERT_TRUE(spot.found);
	EXPECT_TRUE(grid.HasWallBetween(threat, spot.place)) << "the enemy reloads in plain view";
}

TEST(CoverSpotTest, TheSpotIsWalkable)
{
	LevelGrid grid = GridOf(PILLAR);

	Spot spot = Ask(grid, 3, 2, 1, 2);

	ASSERT_TRUE(spot.found);
	EXPECT_TRUE(grid.IsPassable(spot.column, spot.row));
}

TEST(CoverSpotTest, AnOpenHallHasNoCoverAtAll)
{
	LevelGrid grid = GridOf(OPEN_HALL);

	Spot spot = Ask(grid, 4, 2, 1, 2);

	EXPECT_FALSE(spot.found) << "found cover in an empty room";
}

TEST(CoverSpotTest, AWalledOffRoomIsNoCoverBecauseItCannotBeReached)
{
	LevelGrid grid = GridOf(SEALED);

	Spot spot = Ask(grid, 4, 2, 1, 2);

	EXPECT_FALSE(spot.found) << "the enemy hides in a room it cannot walk to";
}

TEST(CoverSpotTest, TheNearestCoverWins)
{
	LevelGrid grid = GridOf(PILLAR);

	Spot spot = Ask(grid, 3, 2, 1, 2);

	ASSERT_TRUE(spot.found);

	PathField field;
	field.Build(grid, 3, 2);

	Vector2Df threat = grid.ToWorld(1, 2);
	for (int row = 1; row <= 3; row++)
	{
		for (int column = 1; column <= 7; column++)
		{
			if (!grid.IsPassable(column, row) || !grid.HasWallBetween(threat, grid.ToWorld(column, row)))
			{
				continue;
			}

			EXPECT_GE(field.GetDistance(column, row), field.GetDistance(spot.column, spot.row))
				<< "closer cover was passed over at " << column << ";" << row;
		}
	}
}

TEST(CoverSpotTest, TheAnswerIsTheSameEveryTime)
{
	LevelGrid grid = GridOf(PILLAR);

	Spot first = Ask(grid, 3, 2, 1, 2);
	Spot again = Ask(grid, 3, 2, 1, 2);

	ASSERT_TRUE(first.found);
	ASSERT_TRUE(again.found);
	EXPECT_EQ(first.column, again.column);
	EXPECT_EQ(first.row, again.row);
}

TEST(CoverSpotTest, AnEmptyGridIsNotSearched)
{
	LevelGrid grid;
	PathField field;
	Vector2Df spot;

	EXPECT_FALSE(FindCoverSpot(grid, field, {0.f, 0.f}, {100.f, 0.f}, RADIUS, spot));
}
