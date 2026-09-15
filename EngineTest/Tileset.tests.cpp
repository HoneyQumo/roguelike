#include "pch.h"
#include "LevelLoader.h"
#include "Tileset.h"
#include <sstream>

using RoguelikeGame::DEFAULT_TILESET;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::TilesetFilePath;
using RoguelikeGame::TilesetName;
using RoguelikeGame::TilesetTextureName;

namespace
{
	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "tileset");
	}
}

TEST(TilesetTest, NameGoesIntoTheFileName)
{
	EXPECT_EQ(TilesetFilePath("catacombs"), "Resources/Textures/tiles_catacombs.png");
	EXPECT_EQ(TilesetTextureName("catacombs"), "tiles_catacombs");
}

TEST(TilesetTest, LevelWithoutATilesetTakesTheDefaultOne)
{
	EXPECT_EQ(TilesetName(""), DEFAULT_TILESET);
	EXPECT_EQ(TilesetFilePath(""), std::string("Resources/Textures/tiles_") + DEFAULT_TILESET + ".png");
}

TEST(TilesetTest, NameIsLoweredAndCleanedUp)
{
	EXPECT_EQ(TilesetName("CataCombs"), "catacombs");
	EXPECT_EQ(TilesetName("old ruins"), "oldruins");
	EXPECT_EQ(TilesetName("hall_2"), "hall_2");
}

TEST(TilesetTest, NameCanNotClimbOutOfTheTexturesFolder)
{
	EXPECT_EQ(TilesetName("../../secret"), "secret");
	EXPECT_EQ(TilesetFilePath("../../secret"), "Resources/Textures/tiles_secret.png");
}

TEST(TilesetTest, NameOfNothingButSymbolsFallsBackToTheDefault)
{
	EXPECT_EQ(TilesetName("///"), DEFAULT_TILESET);
	EXPECT_EQ(TilesetName("..."), DEFAULT_TILESET);
}

TEST(TilesetTest, DifferentNamesGiveDifferentFiles)
{
	EXPECT_NE(TilesetFilePath("ruins"), TilesetFilePath("catacombs"));
	EXPECT_NE(TilesetTextureName("ruins"), TilesetTextureName("catacombs"));
}

TEST(TilesetTest, LevelFileCarriesTheTileset)
{
	LevelData level = LevelOf("[level]\ntitle Catacombs\ntileset catacombs\nmusic march\nambient drip\n[map]\n###\n#.#\n###\n");

	EXPECT_EQ(level.info.tileset, "catacombs");
	EXPECT_EQ(level.info.music, "march");
	EXPECT_EQ(level.info.ambient, "drip");
}

TEST(TilesetTest, LevelFileWithoutTheseFieldsLeavesThemEmpty)
{
	LevelData level = LevelOf("[level]\ntitle Arena\n[map]\n###\n#.#\n###\n");

	EXPECT_TRUE(level.info.tileset.empty());
	EXPECT_TRUE(level.info.music.empty());
	EXPECT_EQ(TilesetName(level.info.tileset), DEFAULT_TILESET);
}
