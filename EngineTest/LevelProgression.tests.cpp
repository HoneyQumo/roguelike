#include "pch.h"
#include "LevelProgression.h"
#include <sstream>

using RoguelikeGame::LevelCatalog;
using RoguelikeGame::LevelMode;
using RoguelikeGame::LevelStep;
using RoguelikeGame::LevelStepKind;
using RoguelikeGame::ResolveNextLevel;

namespace
{
	LevelCatalog MakeThreeLevels()
	{
		std::istringstream input(
			"[level city]\n"
			"title Gorod\n"
			"file Resources/Levels/test_level.config\n"
			"\n"
			"[level catacombs]\n"
			"title Katakomby\n"
			"file Resources/Levels/catacombs.config\n"
			"\n"
			"[level arena]\n"
			"title Arena\n"
			"file Resources/Levels/arena.config\n");

		return LevelCatalog::Parse(input, "levels.config");
	}
}

TEST(LevelProgressionTest, ExplicitNextIdWinsOverFileOrder)
{
	LevelCatalog levels = MakeThreeLevels();

	LevelStep step = ResolveNextLevel(levels, 0, "arena");

	EXPECT_EQ(step.kind, LevelStepKind::Next);
	EXPECT_EQ(step.index, 2);
}

TEST(LevelProgressionTest, EmptyNextIdTakesTheFollowingLevel)
{
	LevelCatalog levels = MakeThreeLevels();

	LevelStep step = ResolveNextLevel(levels, 0, "");

	EXPECT_EQ(step.kind, LevelStepKind::Next);
	EXPECT_EQ(step.index, 1);
}

TEST(LevelProgressionTest, LastLevelFinishesTheRun)
{
	LevelCatalog levels = MakeThreeLevels();

	LevelStep step = ResolveNextLevel(levels, 2, "");

	EXPECT_EQ(step.kind, LevelStepKind::Finished);
}

TEST(LevelProgressionTest, UnknownNextIdIsReported)
{
	LevelCatalog levels = MakeThreeLevels();

	LevelStep step = ResolveNextLevel(levels, 0, "sewers");

	EXPECT_EQ(step.kind, LevelStepKind::Unknown);
	EXPECT_EQ(step.index, -1);
}

TEST(LevelProgressionTest, ThreeLocationsAreWalkedInOrder)
{
	LevelCatalog levels = MakeThreeLevels();

	LevelStep toCatacombs = ResolveNextLevel(levels, 0, "catacombs");
	ASSERT_EQ(toCatacombs.kind, LevelStepKind::Next);
	EXPECT_EQ(levels.GetAt(toCatacombs.index)->id, "catacombs");

	LevelStep toArena = ResolveNextLevel(levels, toCatacombs.index, "arena");
	ASSERT_EQ(toArena.kind, LevelStepKind::Next);
	EXPECT_EQ(levels.GetAt(toArena.index)->id, "arena");

	LevelStep afterArena = ResolveNextLevel(levels, toArena.index, "");
	EXPECT_EQ(afterArena.kind, LevelStepKind::Finished);
}

TEST(LevelProgressionTest, EmptyCatalogFinishesAtOnce)
{
	std::istringstream input("");
	LevelCatalog levels = LevelCatalog::Parse(input, "levels.config");

	LevelStep step = ResolveNextLevel(levels, 0, "");

	EXPECT_EQ(step.kind, LevelStepKind::Finished);
}

namespace
{
	// Сюжет, за ним карта режима и отладочная - ровно та раскладка, на которой ломалась победа.
	LevelCatalog MakeMixedCatalog()
	{
		std::istringstream input(
			"[level prison]\n"
			"file a.config\n"
			"\n"
			"[level street]\n"
			"file b.config\n"
			"\n"
			"[level pit]\n"
			"mode arena\n"
			"file c.config\n"
			"\n"
			"[level scratch]\n"
			"mode test\n"
			"file d.config\n");

		return LevelCatalog::Parse(input, "levels.config");
	}
}

TEST(LevelProgressionTest, TheDefaultStepSkipsLevelsOfAnotherKind)
{
	LevelCatalog levels = MakeMixedCatalog();

	LevelStep step = ResolveNextLevel(levels, 0, "");

	EXPECT_EQ(step.kind, LevelStepKind::Next);
	EXPECT_EQ(step.index, 1);
}

// Главный симптом бага: за последней сюжетной локацией стоят чужие карты, и забег не кончался.
TEST(LevelProgressionTest, TheLastStoryLevelFinishesTheRunEvenWithOtherMapsBelow)
{
	LevelCatalog levels = MakeMixedCatalog();

	LevelStep step = ResolveNextLevel(levels, 1, "");

	EXPECT_EQ(step.kind, LevelStepKind::Finished) << "\u0441\u044e\u0436\u0435\u0442 \u0443\u0442\u0451\u043a \u0432 \u0447\u0443\u0436\u0443\u044e \u043a\u0430\u0440\u0442\u0443";
}

TEST(LevelProgressionTest, EachKindWalksItsOwnChain)
{
	LevelCatalog levels = MakeMixedCatalog();

	EXPECT_EQ(ResolveNextLevel(levels, 2, "").kind, LevelStepKind::Finished);
	EXPECT_EQ(ResolveNextLevel(levels, 3, "").kind, LevelStepKind::Finished);
}

// Явный next - это приказ, он сильнее вида: сцена может увести куда угодно.
TEST(LevelProgressionTest, AnExplicitNextStillReachesAnyKind)
{
	LevelCatalog levels = MakeMixedCatalog();

	LevelStep step = ResolveNextLevel(levels, 0, "pit");

	EXPECT_EQ(step.kind, LevelStepKind::Next);
	EXPECT_EQ(step.index, 2);
}
