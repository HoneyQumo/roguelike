#include "pch.h"
#include "FogFormat.h"
#include "GameSettings.h"
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

// Пока своя легенда затирала стандартную, эта карта была открытым полем с одной дверью,
// и тест проверял дверь, а не стену.
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
TEST(FogLightTest, LightFallsFromThePlayerToTheEdge)
{
	LevelGrid grid = Open(PILLAR_ROOM, 12);

	EXPECT_TRUE(Reveal(grid, 12, 10));

	float atFeet = FogOfWar::Current().GetLight(12, 10);
	float nearby = FogOfWar::Current().GetLight(15, 10);
	float far = FogOfWar::Current().GetLight(22, 10);

	EXPECT_FLOAT_EQ(atFeet, 1.f);
	EXPECT_GT(nearby, far) << "дальняя клетка светит не слабее ближней";
	EXPECT_LT(far, 1.f) << "затухания нет вовсе";
}

TEST(FogLightTest, TheDimmestVisibleCellIsStillBrighterThanMemory)
{
	LevelGrid grid = Open(PILLAR_ROOM, 12);

	EXPECT_TRUE(Reveal(grid, 12, 10));

	for (int column = 1; column < grid.GetWidth() - 1; column++)
	{
		if (FogOfWar::Current().GetState(column, 10) != FogState::Seen)
		{
			continue;
		}

		EXPECT_GE(FogOfWar::Current().GetLight(column, 10), RoguelikeGame::FOG_KNOWN_LIGHT)
			<< "видимая клетка " << column << " темнее запомненной";
	}
}

TEST(FogLightTest, MemoryIsDimmerThanWhatIsSeenNow)
{
	LevelGrid grid = Open(PILLAR_ROOM, 7);

	EXPECT_TRUE(Reveal(grid, 4, 10));
	EXPECT_TRUE(Reveal(grid, 20, 10));

	ASSERT_EQ(FogOfWar::Current().GetState(4, 10), FogState::Known);

	EXPECT_FLOAT_EQ(FogOfWar::Current().GetLight(4, 10), RoguelikeGame::FOG_KNOWN_LIGHT);
	EXPECT_GT(FogOfWar::Current().GetLight(20, 10), RoguelikeGame::FOG_KNOWN_LIGHT);
}

TEST(FogLightTest, WhatWasNeverSeenHasNoLight)
{
	LevelGrid grid = Open(PILLAR_ROOM, 7);

	EXPECT_TRUE(Reveal(grid, 12, 10));

	ASSERT_EQ(FogOfWar::Current().GetState(1, 1), FogState::Unseen);
	EXPECT_FLOAT_EQ(FogOfWar::Current().GetLight(1, 1), 0.f);
}

// Градиент внутри тайла живёт только тем, что углы знают про соседей.
TEST(FogLightTest, ACornerIsTheAverageOfTheFourCellsThatShareIt)
{
	LevelGrid grid = Open(PILLAR_ROOM, 7);

	EXPECT_TRUE(Reveal(grid, 12, 10));

	float corner = FogOfWar::Current().GetCornerLight(12, 10);
	float byHand = 0.25f * (FogOfWar::Current().GetLight(11, 9) + FogOfWar::Current().GetLight(12, 9)
		+ FogOfWar::Current().GetLight(11, 10) + FogOfWar::Current().GetLight(12, 10));

	EXPECT_FLOAT_EQ(corner, byHand);
}

TEST(FogLightTest, ACornerOnTheEdgeOfSightSitsBetweenLightAndDark)
{
	LevelGrid grid = Open(PILLAR_ROOM, 7);

	EXPECT_TRUE(Reveal(grid, 12, 10));

	int edge = 0;
	for (int column = 12; column < grid.GetWidth(); column++)
	{
		if (FogOfWar::Current().GetState(column, 10) == FogState::Unseen)
		{
			edge = column;
			break;
		}
	}

	ASSERT_GT(edge, 12) << "обзор нигде не кончается";

	float corner = FogOfWar::Current().GetCornerLight(edge, 10);

	EXPECT_GT(corner, 0.f) << "край обрезан насухо, градиента нет";
	EXPECT_LT(corner, 1.f);
}


// Луч до центра клетки шёл туда и обратно по разным клеткам: игрок мог видеть врага,
// который его не видит. Для игры, где прячутся, это нечестно.
TEST(FogSymmetryTest, IfThePlayerSeesACellThenThatCellSeesThePlayer)
{
	LevelGrid grid = GridOf(PILLAR_ROOM);
	int radius = 7;

	int checked = 0;

	for (int row = 1; row < grid.GetHeight() - 1; row++)
	{
		for (int column = 1; column < grid.GetWidth() - 1; column++)
		{
			if (!grid.IsPassable(column, row))
			{
				continue;
			}

			FogOfWar::Reset(grid.GetWidth(), grid.GetHeight(), radius);
			Reveal(grid, 12, 10);
			bool seesIt = FogOfWar::Current().GetState(column, row) == FogState::Seen;

			FogOfWar::Reset(grid.GetWidth(), grid.GetHeight(), radius);
			Reveal(grid, column, row);
			bool seesBack = FogOfWar::Current().GetState(12, 10) == FogState::Seen;

			EXPECT_EQ(seesIt, seesBack)
				<< "обзор несимметричен между (12,10) и (" << column << "," << row << ")";
			checked++;
		}
	}

	EXPECT_GT(checked, 100) << "проверено слишком мало клеток";
}

// На мосту карта в сотни тайлов, а видно за раз сотню клеток.
TEST(FogSymmetryTest, OneRevealDoesNotDependOnTheSizeOfTheMap)
{
	const std::string SMALL =
		"[map]\n"
		"###############\n"
		"#.............#\n"
		"#.............#\n"
		"#.............#\n"
		"###############\n";

	std::string wide = "[map]\n";
	for (int row = 0; row < 5; row++)
	{
		std::string line(300, row == 0 || row == 4 ? '#' : '.');
		line[0] = '#';
		line[line.size() - 1] = '#';
		wide += line + "\n";
	}

	LevelGrid small = Open(SMALL, 7);
	Reveal(small, 7, 2);
	int seenSmall = FogOfWar::Current().Count(FogState::Seen);

	LevelGrid big = Open(wide, 7);
	Reveal(big, 7, 2);
	int seenBig = FogOfWar::Current().Count(FogState::Seen);

	EXPECT_EQ(seenSmall, seenBig) << "обзор зависит от размера карты";
}

TEST(FogSymmetryTest, TheEdgeOfSightIsEvenOnOpenGround)
{
	std::string open = "[map]\n";
	for (int row = 0; row < 21; row++)
	{
		std::string line(21, row == 0 || row == 20 ? '#' : '.');
		line[0] = '#';
		line[20] = '#';
		open += line + "\n";
	}

	LevelGrid grid = Open(open, 7);
	Reveal(grid, 10, 10);

	// На чистом месте обзор обязан быть одинаков во все четыре стороны.
	for (int step = 1; step <= 7; step++)
	{
		bool right = FogOfWar::Current().GetState(10 + step, 10) == FogState::Seen;
		bool left = FogOfWar::Current().GetState(10 - step, 10) == FogState::Seen;
		bool down = FogOfWar::Current().GetState(10, 10 + step) == FogState::Seen;
		bool up = FogOfWar::Current().GetState(10, 10 - step) == FogState::Seen;

		EXPECT_EQ(right, left) << "шаг " << step;
		EXPECT_EQ(right, down) << "шаг " << step;
		EXPECT_EQ(right, up) << "шаг " << step;
	}
}


// Лучи оставляли четыре угла комнаты чёрными навсегда: диагональный луч до угла
// всегда резал две стены, с какой бы проходимой клетки на него ни смотрели.
TEST(FogSymmetryTest, WalkingTheWholeRoomOpensEveryCellOfIt)
{
	LevelGrid grid = GridOf(PILLAR_ROOM);

	std::vector<bool> everSeen(static_cast<std::size_t>(grid.GetWidth()) * grid.GetHeight(), false);
	int standings = 0;

	for (int row = 0; row < grid.GetHeight(); row++)
	{
		for (int column = 0; column < grid.GetWidth(); column++)
		{
			if (!grid.IsPassable(column, row))
			{
				continue;
			}

			standings++;
			FogOfWar::Reset(grid.GetWidth(), grid.GetHeight(), 7);
			Reveal(grid, column, row);

			for (int cellRow = 0; cellRow < grid.GetHeight(); cellRow++)
			{
				for (int cellColumn = 0; cellColumn < grid.GetWidth(); cellColumn++)
				{
					if (FogOfWar::Current().GetState(cellColumn, cellRow) == FogState::Seen)
					{
						everSeen[static_cast<std::size_t>(cellRow) * grid.GetWidth() + cellColumn] = true;
					}
				}
			}
		}
	}

	ASSERT_GT(standings, 100);

	int blind = 0;
	for (bool seen : everSeen)
	{
		blind += seen ? 0 : 1;
	}

	EXPECT_EQ(blind, 0) << blind << " клеток не открываются ни с одной проходимой клетки";
}


// Своя легенда объявляет только дверь, а стены и пол берёт из стандартной.
TEST(FogOfWarTest, TheTwoRoomsMapReallyHasWalls)
{
	LevelGrid grid = GridOf(TWO_ROOMS);

	EXPECT_TRUE(grid.BlocksSight(5, 1)) << "стена между комнатами обзор не закрывает";
	EXPECT_TRUE(grid.BlocksSight(0, 0)) << "внешняя стена обзор не закрывает";
	EXPECT_TRUE(grid.IsPassable(2, 2)) << "пол не проходим";
	EXPECT_TRUE(grid.BlocksSight(5, 3)) << "закрытая дверь обзор не закрывает";
}

// Стена без двери: раньше такой проверки не было вовсе.
TEST(FogOfWarTest, ASolidWallHidesTheRoomBehindIt)
{
	const std::string SPLIT =
		"[legend]\n"
		"+ Door:door_test\n"
		"[map]\n"
		"###########\n"
		"#....#....#\n"
		"#....#....#\n"
		"#....#....#\n"
		"###########\n";

	LevelGrid grid = Open(SPLIT, 7);

	EXPECT_TRUE(Reveal(grid, 2, 2));

	EXPECT_EQ(FogOfWar::Current().GetState(2, 2), FogState::Seen);
	EXPECT_EQ(FogOfWar::Current().GetState(5, 2), FogState::Seen) << "стена перед игроком тёмная";
	EXPECT_EQ(FogOfWar::Current().GetState(7, 2), FogState::Unseen) << "видно сквозь сплошную стену";
}
