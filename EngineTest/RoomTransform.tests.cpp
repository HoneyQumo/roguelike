#include "pch.h"
#include "LevelLoader.h"
#include "RoomTransform.h"
#include <sstream>

using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::TileType;
using RoguelikeGame::TransformRoom;

namespace
{
	LevelData RoomOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "room");
	}

	const std::string SHAPE =
		"[level]\n"
		"kind corridor\n"
		"tileset catacombs\n"
		"music dark\n"
		"ambient drip\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"p Item:potion_small\n"
		"c Prop:crate_ammo\n"
		"1 Watch:guard\n"
		"[map]\n"
		"#######\n"
		"#.p...#\n"
		"#.###.#\n"
		"#c...1#\n"
		"##.####\n";

	std::string Draw(const LevelData& room)
	{
		std::string picture;
		for (int row = 0; row < room.height; row++)
		{
			for (int column = 0; column < room.width; column++)
			{
				TileType tile = room.tiles[row][column];
				picture += tile == TileType::Wall ? '#' : (tile == TileType::Floor ? '.' : ' ');
			}
			picture += '|';
		}

		return picture;
	}
}

TEST(RoomTransformTest, RoomWithoutTurnsStaysTheSame)
{
	LevelData room = RoomOf(SHAPE);
	LevelData same = TransformRoom(room, 0, false);

	EXPECT_EQ(same.width, room.width);
	EXPECT_EQ(same.height, room.height);
	EXPECT_EQ(Draw(same), Draw(room));
}

TEST(RoomTransformTest, QuarterTurnSwapsTheSides)
{
	LevelData room = RoomOf(SHAPE);
	LevelData turned = TransformRoom(room, 1, false);

	EXPECT_EQ(turned.width, room.height);
	EXPECT_EQ(turned.height, room.width);
}

TEST(RoomTransformTest, FourQuartersComeBackToTheStart)
{
	LevelData room = RoomOf(SHAPE);
	LevelData turned = TransformRoom(room, 4, false);

	EXPECT_EQ(Draw(turned), Draw(room));
}

TEST(RoomTransformTest, TurningTwiceByTwoIsTheSameAsFour)
{
	LevelData room = RoomOf(SHAPE);
	LevelData once = TransformRoom(TransformRoom(room, 2, false), 2, false);

	EXPECT_EQ(Draw(once), Draw(room));
}

TEST(RoomTransformTest, MirrorTwiceComesBack)
{
	LevelData room = RoomOf(SHAPE);
	LevelData twice = TransformRoom(TransformRoom(room, 0, true), 0, true);

	EXPECT_EQ(Draw(twice), Draw(room));
}

TEST(RoomTransformTest, MirrorIsNotTheSameAsTurning)
{
	LevelData room = RoomOf(SHAPE);

	EXPECT_NE(Draw(TransformRoom(room, 0, true)), Draw(room));
	EXPECT_NE(Draw(TransformRoom(room, 0, true)), Draw(TransformRoom(room, 2, false)));
}

TEST(RoomTransformTest, CornerGoesWhereTheTurnPutsIt)
{
	LevelData room = RoomOf(SHAPE);
	LevelData turned = TransformRoom(room, 1, false);

	EXPECT_EQ(room.tiles[1][1], TileType::Floor);
	EXPECT_EQ(turned.tiles[1][turned.width - 2], TileType::Floor);
}

TEST(RoomTransformTest, ThingsInsideTurnWithTheRoom)
{
	LevelData room = RoomOf(SHAPE);
	ASSERT_EQ(room.items.size(), 1u);
	ASSERT_EQ(room.props.size(), 1u);
	ASSERT_EQ(room.patrols.size(), 1u);

	LevelData turned = TransformRoom(room, 1, false);
	ASSERT_EQ(turned.items.size(), 1u);
	ASSERT_EQ(turned.props.size(), 1u);
	ASSERT_EQ(turned.patrols.size(), 1u);

	EXPECT_EQ(turned.items[0].column, room.height - 1 - room.items[0].row);
	EXPECT_EQ(turned.items[0].row, room.items[0].column);
	EXPECT_EQ(turned.props[0].column, room.height - 1 - room.props[0].row);
	EXPECT_EQ(turned.patrols[0].column, room.height - 1 - room.patrols[0].row);
	EXPECT_EQ(turned.patrols[0].routeId, room.patrols[0].routeId);
	EXPECT_TRUE(turned.patrols[0].isWatch);
}

TEST(RoomTransformTest, ThingsStayOnTheirTiles)
{
	LevelData room = RoomOf(SHAPE);

	for (int quarters = 0; quarters < 4; quarters++)
	{
		for (int mirror = 0; mirror < 2; mirror++)
		{
			LevelData moved = TransformRoom(room, quarters, mirror != 0);

			ASSERT_EQ(moved.items.size(), 1u);
			EXPECT_EQ(moved.tiles[moved.items[0].row][moved.items[0].column], TileType::Floor)
				<< "quarters " << quarters << " mirror " << mirror;
			EXPECT_EQ(moved.tiles[moved.props[0].row][moved.props[0].column], TileType::Floor);
			EXPECT_EQ(moved.tiles[moved.patrols[0].row][moved.patrols[0].column], TileType::Floor);
		}
	}
}

TEST(RoomTransformTest, RoomSettingsSurviveTheTurn)
{
	LevelData room = RoomOf(SHAPE);
	LevelData turned = TransformRoom(room, 3, true);

	EXPECT_EQ(turned.info.kind, "corridor");
	EXPECT_EQ(turned.info.tileset, "catacombs");
	EXPECT_EQ(turned.info.music, "dark");
	EXPECT_EQ(turned.info.ambient, "drip");
}
