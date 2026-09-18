#include "pch.h"
#include "LevelCatalog.h"
#include <sstream>

using RoguelikeGame::LevelCatalog;
using RoguelikeGame::LevelMode;

namespace
{
	LevelCatalog ParseLevels(const std::string& text)
	{
		std::istringstream input(text);
		return LevelCatalog::Parse(input, "test");
	}

	const std::string TWO_LEVELS =
		"[level city]\n"
		"title Gorod\n"
		"file Resources/Levels/city.config\n"
		"\n"
		"[level arena]\n"
		"title Arena\n"
		"file Resources/Levels/arena.config\n";
}

TEST(LevelCatalogTests, ParsesLevelsInFileOrder)
{
	LevelCatalog catalog = ParseLevels(TWO_LEVELS);

	ASSERT_EQ(catalog.Size(), 2u);
	EXPECT_EQ(catalog.GetAt(0)->id, "city");
	EXPECT_EQ(catalog.GetAt(1)->id, "arena");
	EXPECT_EQ(catalog.GetFirst()->title, "Gorod");
	EXPECT_EQ(catalog.GetAt(1)->filePath, "Resources/Levels/arena.config");
}

TEST(LevelCatalogTests, FindsLevelByIdAndIndex)
{
	LevelCatalog catalog = ParseLevels(TWO_LEVELS);

	EXPECT_EQ(catalog.Find("arena")->title, "Arena");
	EXPECT_EQ(catalog.IndexOf("arena"), 1);
	EXPECT_EQ(catalog.Find("no_such_level"), nullptr);
	EXPECT_EQ(catalog.IndexOf("no_such_level"), -1);
}

TEST(LevelCatalogTests, EmptyCatalogHasNoFirstLevel)
{
	LevelCatalog catalog = ParseLevels("; only a comment\n");

	EXPECT_TRUE(catalog.IsEmpty());
	EXPECT_EQ(catalog.GetFirst(), nullptr);
	EXPECT_EQ(catalog.GetAt(0), nullptr);
}

TEST(LevelCatalogTests, LevelWithoutFileIsRejected)
{
	EXPECT_THROW(ParseLevels("[level broken]\ntitle Broken\n"), std::runtime_error);
}

TEST(LevelCatalogTests, DuplicateIdIsRejected)
{
	EXPECT_THROW(ParseLevels(TWO_LEVELS + "\n[level city]\nfile again.config\n"), std::runtime_error);
}

TEST(LevelCatalogTests, BlockWithoutIdIsRejected)
{
	EXPECT_THROW(ParseLevels("[level ]\nfile a.config\n"), std::runtime_error);
}

TEST(LevelCatalogTests, FieldOutsideOfBlockIsRejected)
{
	EXPECT_THROW(ParseLevels("file lost.config\n[level city]\nfile a.config\n"), std::runtime_error);
}

TEST(LevelCatalogTests, UnknownFieldIsIgnored)
{
	LevelCatalog catalog = ParseLevels("[level city]\nfile a.config\nmusic theme.ogg\n");

	ASSERT_EQ(catalog.Size(), 1u);
	EXPECT_EQ(catalog.GetFirst()->filePath, "a.config");
}

// Вид не указан - локация сюжетная: реестр не должен требовать строку от каждой карты.
TEST(LevelCatalogTests, ALevelIsCampaignUnlessItSaysOtherwise)
{
	LevelCatalog catalog = ParseLevels(TWO_LEVELS);

	EXPECT_EQ(catalog.GetAt(0)->mode, LevelMode::Campaign);
	EXPECT_EQ(catalog.GetAt(1)->mode, LevelMode::Campaign);
}

TEST(LevelCatalogTests, TheModeIsReadFromTheRegistry)
{
	LevelCatalog catalog = ParseLevels(
		"[level story]\n"
		"file a.config\n"
		"\n"
		"[level pit]\n"
		"mode arena\n"
		"file b.config\n"
		"\n"
		"[level scratch]\n"
		"mode test\n"
		"file c.config\n");

	EXPECT_EQ(catalog.Find("story")->mode, LevelMode::Campaign);
	EXPECT_EQ(catalog.Find("pit")->mode, LevelMode::Arena);
	EXPECT_EQ(catalog.Find("scratch")->mode, LevelMode::Test);
}

// Опечатка в реестре не должна ронять игру и не должна тихо выкидывать локацию из сюжета.
TEST(LevelCatalogTests, AnUnknownModeFallsBackToCampaign)
{
	LevelCatalog catalog = ParseLevels("[level city]\nmode sandbox\nfile a.config\n");

	ASSERT_EQ(catalog.Size(), 1u);
	EXPECT_EQ(catalog.GetFirst()->mode, LevelMode::Campaign);
}

TEST(LevelCatalogTests, TheWalkGoesThroughOneKindOnly)
{
	LevelCatalog catalog = ParseLevels(
		"[level scratch]\n"
		"mode test\n"
		"file a.config\n"
		"\n"
		"[level first]\n"
		"file b.config\n"
		"\n"
		"[level pit]\n"
		"mode arena\n"
		"file c.config\n"
		"\n"
		"[level second]\n"
		"file d.config\n");

	EXPECT_EQ(catalog.FirstIndex(LevelMode::Campaign), 1) << "\u0437\u0430\u0431\u0435\u0433 \u043d\u0430\u0447\u0430\u043b\u0441\u044f \u043d\u0435 \u0441 \u0441\u044e\u0436\u0435\u0442\u043d\u043e\u0439 \u043b\u043e\u043a\u0430\u0446\u0438\u0438";
	EXPECT_EQ(catalog.NextIndex(LevelMode::Campaign, 1), 3) << "\u0448\u0430\u0433 \u0437\u0430\u0448\u0451\u043b \u043d\u0430 \u0447\u0443\u0436\u0443\u044e \u043a\u0430\u0440\u0442\u0443";
	EXPECT_EQ(catalog.NextIndex(LevelMode::Campaign, 3), -1) << "\u0446\u0435\u043f\u043e\u0447\u043a\u0430 \u043d\u0435 \u043a\u043e\u043d\u0447\u0430\u0435\u0442\u0441\u044f";
	EXPECT_EQ(catalog.FirstIndex(LevelMode::Arena), 2);
	EXPECT_EQ(catalog.NextIndex(LevelMode::Arena, 2), -1);
}

// Индекс приходит из состояния сцены и может быть любым - обход не должен уходить за начало.
TEST(LevelCatalogTests, AStrayIndexDoesNotWalkOffTheStart)
{
	LevelCatalog catalog = ParseLevels(TWO_LEVELS);

	EXPECT_EQ(catalog.NextIndex(LevelMode::Campaign, -7), 0);
	EXPECT_EQ(catalog.FirstIndex(LevelMode::Test), -1);
}
