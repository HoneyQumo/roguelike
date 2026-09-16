#include "pch.h"
#include "LevelLoader.h"
#include "ActAssembler.h"
#include "LevelCatalog.h"
#include "ProjectFiles.h"
#include "Tileset.h"
#include <SFML/Graphics/Image.hpp>
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

namespace
{
	constexpr unsigned int ATLAS_WIDTH = 1024u;
	// Пол, стены, разметка и вода - по строке на каждое, у всех тайлсетов одинаково.
	constexpr unsigned int ATLAS_HEIGHT = 256u;

	class ShippedTilesetsTest : public ProjectFiles::Test
	{
	};
}

TEST(TilesetTest, ActOneSetsHaveTheirOwnFiles)
{
	EXPECT_EQ(TilesetFilePath("prison"), "Resources/Textures/tiles_prison.png");
	EXPECT_EQ(TilesetFilePath("street"), "Resources/Textures/tiles_street.png");
	EXPECT_EQ(TilesetFilePath("bridge"), "Resources/Textures/tiles_bridge.png");
}

TEST_F(ShippedTilesetsTest, EveryAtlasHasTheExpectedSize)
{
	ASSERT_TRUE(isFound) << previous.string();

	int found = 0;
	for (const auto& entry : std::filesystem::directory_iterator("Resources/Textures"))
	{
		std::string name = entry.path().filename().string();
		if (name.rfind("tiles_", 0) != 0)
		{
			continue;
		}

		sf::Image atlas;
		ASSERT_TRUE(atlas.loadFromFile(entry.path().string())) << name;
		EXPECT_EQ(atlas.getSize().x, ATLAS_WIDTH) << name;
		EXPECT_EQ(atlas.getSize().y, ATLAS_HEIGHT) << name;
		found++;
	}

	EXPECT_GE(found, 5);
}

TEST_F(ShippedTilesetsTest, EveryLevelAsksForATilesetThatExists)
{
	ASSERT_TRUE(isFound) << previous.string();

	RoguelikeGame::LevelCatalog catalog = RoguelikeGame::LevelCatalog::Load("Resources/Levels/levels.config");
	ASSERT_FALSE(catalog.IsEmpty());

	for (const RoguelikeGame::LevelEntry& entry : catalog)
	{
		LevelData level = entry.isAct
			? RoguelikeGame::LoadAct(entry.filePath)
			: LevelLoader::Load(entry.filePath);

		EXPECT_TRUE(std::filesystem::exists(TilesetFilePath(level.info.tileset)))
			<< entry.id << " asks for " << TilesetName(level.info.tileset);
	}
}
