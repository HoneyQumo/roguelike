#include "pch.h"
#include "FogFormat.h"
#include "FogOfWar.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include "SightRules.h"
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

	// Комната со столбами: у стен есть куски под любым углом к игроку.
	const std::string PILLAR_ROOM =
		"[map]\n"
		"##########################\n"
		"#........................#\n"
		"#........................#\n"
		"#....####....#####.......#\n"
		"#....#..#....#...#.......#\n"
		"#....#..#....#...#.......#\n"
		"#....####....#####.......#\n"
		"#........................#\n"
		"#........................#\n"
		"#....##..........##......#\n"
		"#........................#\n"
		"#........................#\n"
		"##########################\n";

	LevelGrid GridOf(const std::string& map)
	{
		std::istringstream input(map);

		return LevelGrid::Build(LevelLoader::Parse(input, "fog"));
	}

	struct Coverage
	{
		int walls = 0;
		int wallsLit = 0;
		int floors = 0;
		int floorsLit = 0;

		int WallPart() const { return walls > 0 ? 100 * wallsLit / walls : 0; }
		int FloorPart() const { return floors > 0 ? 100 * floorsLit / floors : 0; }
	};

	Coverage CoverageAround(const LevelGrid& grid, int column, int row, int radius)
	{
		Coverage found;

		for (int cellRow = 0; cellRow < grid.GetHeight(); cellRow++)
		{
			for (int cellColumn = 0; cellColumn < grid.GetWidth(); cellColumn++)
			{
				int alongColumns = cellColumn - column;
				int alongRows = cellRow - row;
				if (alongColumns * alongColumns + alongRows * alongRows > radius * radius)
				{
					continue;
				}

				bool isLit = FogOfWar::Current().GetState(cellColumn, cellRow) == FogState::Seen;

				if (grid.BlocksSight(cellColumn, cellRow))
				{
					found.walls++;
					found.wallsLit += isLit ? 1 : 0;
				}
				else
				{
					found.floors++;
					found.floorsLit += isLit ? 1 : 0;
				}
			}
		}

		return found;
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


// Луч до центра стены под косым углом задевал соседнюю стену: в радиусе 12 светилась
// четверть стен против шестидесяти процентов пола, и комната читалась лужей пола без границ.
TEST(FogWallLightingTest, WallsAreLitNoWorseThanTheFloorTheyStandOn)
{
	LevelGrid grid = Open(PILLAR_ROOM, 12);

	EXPECT_TRUE(Reveal(grid, 12, 10));

	Coverage found = CoverageAround(grid, 12, 10, 12);

	EXPECT_GE(found.WallPart(), found.FloorPart())
		<< "стены подсвечены хуже пола: " << found.wallsLit << "/" << found.walls
		<< " против " << found.floorsLit << "/" << found.floors;
}

TEST(FogWallLightingTest, TheWholeWallInFrontOfThePlayerIsLit)
{
	LevelGrid grid = Open(PILLAR_ROOM, 7);

	EXPECT_TRUE(Reveal(grid, 12, 10));

	for (int column = 6; column <= 18; column++)
	{
		EXPECT_EQ(FogOfWar::Current().GetState(column, 12), FogState::Seen)
			<< "тайл " << column << " нижней стены остался тёмным";
	}
}

TEST(FogWallLightingTest, AWallBehindTheCornerStaysDark)
{
	LevelGrid grid = Open(PILLAR_ROOM, 7);

	EXPECT_TRUE(Reveal(grid, 12, 10));

	// Дальняя от игрока стенка столба: пол за столбом не виден, значит и она не должна.
	EXPECT_EQ(FogOfWar::Current().GetState(6, 3), FogState::Unseen) << "стена засветилась сквозь столб";
}

// Иначе стена между двумя комнатами загоралась бы из той, куда игрок не смотрит.
TEST(FogWallLightingTest, AWallIsLitFromThePlayerSideOnly)
{
	const std::string TWO_HALVES =
		"[map]\n"
		"###########\n"
		"#....#....#\n"
		"#....#....#\n"
		"#....#....#\n"
		"###########\n";

	LevelGrid grid = Open(TWO_HALVES, 7);

	EXPECT_TRUE(Reveal(grid, 2, 2));

	EXPECT_EQ(FogOfWar::Current().GetState(5, 2), FogState::Seen) << "стена перед игроком тёмная";
	EXPECT_EQ(FogOfWar::Current().GetState(7, 2), FogState::Unseen) << "видно сквозь стену";
}

TEST(FogWallLightingTest, LightingWallsDoesNotOpenTheFloorBehindThem)
{
	LevelGrid grid = Open(PILLAR_ROOM, 12);

	EXPECT_TRUE(Reveal(grid, 12, 10));

	// Пол внутри столба закрыт со всех сторон и обязан остаться закрытым.
	EXPECT_EQ(FogOfWar::Current().GetState(6, 4), FogState::Unseen);
	EXPECT_EQ(FogOfWar::Current().GetState(7, 5), FogState::Unseen);
}

TEST(FogWallLightingTest, StepsLeadTowardTheViewerAndNowhereElse)
{
	RoguelikeGame::SightStep steps[2];

	EXPECT_EQ(RoguelikeGame::StepsTowardViewer(0, 0, steps), 0) << "под ногами шагать некуда";

	ASSERT_EQ(RoguelikeGame::StepsTowardViewer(3, 0, steps), 1);
	EXPECT_EQ(steps[0].column, -1);
	EXPECT_EQ(steps[0].row, 0);

	ASSERT_EQ(RoguelikeGame::StepsTowardViewer(0, -4, steps), 1);
	EXPECT_EQ(steps[0].column, 0);
	EXPECT_EQ(steps[0].row, 1);

	ASSERT_EQ(RoguelikeGame::StepsTowardViewer(-2, 5, steps), 2) << "по диагонали шагов два, по одному на ось";
	EXPECT_EQ(steps[0].column, 1);
	EXPECT_EQ(steps[1].row, -1);
}
