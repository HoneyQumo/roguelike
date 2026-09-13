#include "pch.h"
#include "LevelCatalog.h"
#include <sstream>

using RoguelikeGame::LevelCatalog;

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
