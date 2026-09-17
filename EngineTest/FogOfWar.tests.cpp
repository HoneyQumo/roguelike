#include "pch.h"
#include "FogFormat.h"
#include "FogOfWar.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include <sstream>

using RoguelikeGame::FogOfWar;
using RoguelikeGame::FogState;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::ParseFogRadius;

namespace
{
	constexpr int RADIUS = 3;

	// Коридор в одну клетку: видно вдоль, и дальше радиуса не видно ничего.
	const std::string CORRIDOR =
		"[map]\n"
		"#############\n"
		"#...........#\n"
		"#############\n";

	// Две комнаты через дверь: пока она закрыта, за неё не видно.
	const std::string TWO_ROOMS =
		"[legend]\n"
		"+ Door:door_test\n"
		"[map]\n"
		"###########\n"
		"#....#....#\n"
		"#....#....#\n"
		"#....+....#\n"
		"#....#....#\n"
		"#....#....#\n"
		"###########\n";

	LevelGrid GridOf(const std::string& map)
	{
		std::istringstream input(map);

		return LevelGrid::Build(LevelLoader::Parse(input, "fog"));
	}

	LevelGrid Open(const std::string& map, int radius)
	{
		LevelGrid grid = GridOf(map);
		FogOfWar::Reset(grid.GetWidth(), grid.GetHeight(), radius);

		return grid;
	}

	bool Reveal(const LevelGrid& grid, int column, int row)
	{
		return FogOfWar::Current().Reveal(grid, grid.ToWorld(column, row));
	}
}

TEST(FogOfWarTest, AFreshLevelIsAllUnseen)
{
	LevelGrid grid = Open(CORRIDOR, RADIUS);

	EXPECT_EQ(FogOfWar::Current().Count(FogState::Unseen), grid.GetWidth() * grid.GetHeight());
	EXPECT_EQ(FogOfWar::Current().Count(FogState::Seen), 0);
}

TEST(FogOfWarTest, WhatIsAroundThePlayerOpens)
{
	LevelGrid grid = Open(CORRIDOR, RADIUS);

	EXPECT_TRUE(Reveal(grid, 1, 1));

	EXPECT_EQ(FogOfWar::Current().GetState(1, 1), FogState::Seen);
	EXPECT_EQ(FogOfWar::Current().GetState(4, 1), FogState::Seen) << "край радиуса не открылся";
}

TEST(FogOfWarTest, BeyondTheRadiusNothingOpens)
{
	LevelGrid grid = Open(CORRIDOR, RADIUS);

	Reveal(grid, 1, 1);

	EXPECT_EQ(FogOfWar::Current().GetState(5, 1), FogState::Unseen) << "радиус не держит";
}

TEST(FogOfWarTest, WhatIsLeftBehindStaysRemembered)
{
	LevelGrid grid = Open(CORRIDOR, RADIUS);

	Reveal(grid, 1, 1);
	Reveal(grid, 10, 1);

	EXPECT_EQ(FogOfWar::Current().GetState(1, 1), FogState::Known) << "пройденное забылось";
	EXPECT_EQ(FogOfWar::Current().GetState(10, 1), FogState::Seen);
}

TEST(FogOfWarTest, AWallHidesWhatIsBehindIt)
{
	LevelGrid grid = Open(TWO_ROOMS, 6);

	Reveal(grid, 2, 3);

	EXPECT_EQ(FogOfWar::Current().GetState(2, 3), FogState::Seen);
	EXPECT_EQ(FogOfWar::Current().GetState(7, 3), FogState::Unseen) << "видно сквозь стену";
}

TEST(FogOfWarTest, AnOpenedDoorLetsTheSightThrough)
{
	LevelGrid grid = Open(TWO_ROOMS, 6);

	Reveal(grid, 2, 3);
	ASSERT_EQ(FogOfWar::Current().GetState(7, 3), FogState::Unseen);

	LevelGrid::SetCurrent(GridOf(TWO_ROOMS));
	LevelGrid::OpenCell(LevelGrid::Current().ToWorld(5, 3));
	Reveal(LevelGrid::Current(), 2, 3);

	EXPECT_EQ(FogOfWar::Current().GetState(7, 3), FogState::Seen) << "открытая дверь ничего не открыла";
}

TEST(FogOfWarTest, AnOpenedCellMovesTheGridVersion)
{
	LevelGrid::SetCurrent(GridOf(TWO_ROOMS));
	unsigned int before = LevelGrid::Current().GetVersion();

	LevelGrid::OpenCell(LevelGrid::Current().ToWorld(5, 3));

	EXPECT_NE(LevelGrid::Current().GetVersion(), before);
}

TEST(FogOfWarTest, StandingStillChangesNothing)
{
	LevelGrid grid = Open(CORRIDOR, RADIUS);

	Reveal(grid, 1, 1);
	unsigned int version = FogOfWar::Current().GetVersion();

	EXPECT_FALSE(Reveal(grid, 1, 1)) << "тот же обзор объявлен новым";
	EXPECT_EQ(FogOfWar::Current().GetVersion(), version) << "полотно перекрасят зря";
}

TEST(FogOfWarTest, ALevelWithoutFogIsVisibleWholeAndCostsNothing)
{
	LevelGrid grid = Open(CORRIDOR, 0);

	EXPECT_FALSE(FogOfWar::Current().IsEnabled());
	EXPECT_EQ(FogOfWar::Current().GetState(10, 1), FogState::Seen);
	EXPECT_FALSE(Reveal(grid, 1, 1));
}

TEST(FogOfWarTest, TheFogKeyReadsWordsAndNumbers)
{
	int radius = -1;

	EXPECT_TRUE(ParseFogRadius("off", radius));
	EXPECT_EQ(radius, 0);

	EXPECT_TRUE(ParseFogRadius("on", radius));
	EXPECT_EQ(radius, RoguelikeGame::FOG_SIGHT_RADIUS);

	EXPECT_TRUE(ParseFogRadius("12", radius));
	EXPECT_EQ(radius, 12);

	EXPECT_FALSE(ParseFogRadius("deep", radius));
	EXPECT_EQ(radius, 12) << "непонятное значение затёрло прежнее";
}
