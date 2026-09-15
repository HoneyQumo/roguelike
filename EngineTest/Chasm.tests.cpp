#include "pch.h"
#include "Chasm.h"
#include "LevelLoader.h"
#include <sstream>

using RoguelikeGame::IsChasmEdge;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;

namespace
{
	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "chasm");
	}

	const std::string BRIDGE =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"+ Door:door_exit\n"
		"[map]\n"
		"#######\n"
		"#     #\n"
		"#@....#\n"
		"#     #\n"
		"#######\n";
}

TEST(ChasmTest, EmptyNextToTheFloorIsAnEdge)
{
	LevelData level = LevelOf(BRIDGE);

	EXPECT_TRUE(IsChasmEdge(level, 2, 1));
	EXPECT_TRUE(IsChasmEdge(level, 2, 3));
}

TEST(ChasmTest, TheFloorItselfIsNeverAnEdge)
{
	LevelData level = LevelOf(BRIDGE);

	EXPECT_FALSE(IsChasmEdge(level, 2, 2));
}

TEST(ChasmTest, EmptyAwayFromTheFloorIsNotAnEdge)
{
	LevelData level = LevelOf(
		"#########\n"
		"#       #\n"
		"#       #\n"
		"#  @    #\n"
		"#########\n");

	EXPECT_FALSE(IsChasmEdge(level, 7, 1));
}

TEST(ChasmTest, EmptyAmongWallsOnlyIsNotAnEdge)
{
	LevelData level = LevelOf(
		"#####\n"
		"## ##\n"
		"#####\n"
		"#@..#\n"
		"#####\n");

	EXPECT_FALSE(IsChasmEdge(level, 2, 1));
}

TEST(ChasmTest, EmptyNextToADoorIsAnEdge)
{
	LevelData level = LevelOf(
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"@ PlayerSpawn\n"
		"+ Door:door_exit\n"
		"[map]\n"
		"#####\n"
		"#@+ #\n"
		"#####\n");

	EXPECT_TRUE(IsChasmEdge(level, 3, 1));
}

TEST(ChasmTest, OutsideTheMapIsNotAnEdge)
{
	LevelData level = LevelOf(
		"#####\n"
		"#@..#\n"
		"#####\n");

	EXPECT_FALSE(IsChasmEdge(level, 9, 9));
	EXPECT_FALSE(IsChasmEdge(level, -1, 0));
}
