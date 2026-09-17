#include "pch.h"
#include "ActAssembler.h"
#include "LevelLoader.h"
#include "ProjectFiles.h"
#include "RoomTransform.h"
#include "TileAtlas.h"
#include <chrono>
#include <iostream>
#include <sstream>

using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::OverlayAt;
using RoguelikeGame::TileAt;
using RoguelikeGame::TileType;

namespace
{
	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "overlay");
	}

	const char* PATCH =
	"[level]\n"
	"kind patch\n"
	"\n"
	"[legend]\n"
	"# Wall\n"
	". Floor\n"
	"~ Water\n"
	"- Line\n"
	"@ PlayerSpawn\n"
	"> Exit\n"
	"\n"
	"[map]\n"
	"#####\n"
	"#@.>#\n"
	"#~~~#\n"
	"#####\n"
	"\n"
	"[overlay]\n"
	"     \n"
	" --- \n"
	"     \n"
	"     \n";

	const char* PLAIN =
	"[level]\n"
	"kind plain\n"
	"\n"
	"[legend]\n"
	"# Wall\n"
	". Floor\n"
	"@ PlayerSpawn\n"
	"> Exit\n"
	"\n"
	"[map]\n"
	"#####\n"
	"#@.>#\n"
	"#####\n";

	// Накладка короче карты: строка кончается раньше, чем ряд клеток.
	const char* RAGGED =
	"[level]\n"
	"kind ragged\n"
	"\n"
	"[legend]\n"
	"# Wall\n"
	". Floor\n"
	"- Line\n"
	"@ PlayerSpawn\n"
	"> Exit\n"
	"\n"
	"[map]\n"
	"#####\n"
	"#@.>#\n"
	"#####\n"
	"\n"
	"[overlay]\n"
	"  -\n";
}

TEST(OverlayFormatTests, WithoutTheSectionThereIsNoUpperLayer)
{
	LevelData level = LevelOf(PLAIN);

	EXPECT_TRUE(level.overlay.empty()) << "an empty layer costs memory on every level";
}

TEST(OverlayFormatTests, TheUpperLayerIsReadWithTheSameLegend)
{
	LevelData level = LevelOf(PATCH);

	ASSERT_FALSE(level.overlay.empty());
	EXPECT_EQ(OverlayAt(level, 1, 1), TileType::Line);
	EXPECT_EQ(OverlayAt(level, 3, 1), TileType::Line);
}

TEST(OverlayFormatTests, ASpaceInTheUpperLayerLeavesTheGroundBare)
{
	LevelData level = LevelOf(PATCH);

	EXPECT_EQ(OverlayAt(level, 0, 1), TileType::Empty);
	EXPECT_EQ(OverlayAt(level, 2, 0), TileType::Empty);
}

TEST(OverlayFormatTests, TheUpperLayerDoesNotTouchTheGround)
{
	LevelData level = LevelOf(PATCH);

	// Под накладкой остаётся ровно то, что было: слой только рисует.
	EXPECT_EQ(TileAt(level, 1, 1), TileType::PlayerSpawn);
	EXPECT_EQ(TileAt(level, 2, 1), TileType::Floor);
	EXPECT_EQ(TileAt(level, 1, 2), TileType::Water);
}

TEST(OverlayFormatTests, TheUpperLayerBringsNoItemsOrDoors)
{
	LevelData level = LevelOf(PATCH);

	EXPECT_TRUE(level.items.empty());
	EXPECT_TRUE(level.doors.empty());
	EXPECT_TRUE(level.props.empty());
}

TEST(OverlayFormatTests, AShortLineIsNotAnError)
{
	LevelData level = LevelOf(RAGGED);

	ASSERT_FALSE(level.overlay.empty());
	EXPECT_EQ(OverlayAt(level, 2, 0), TileType::Line);
	EXPECT_EQ(OverlayAt(level, 4, 0), TileType::Empty) << "reading past the line must be safe";
	EXPECT_EQ(OverlayAt(level, 0, 9), TileType::Empty) << "reading past the layer must be safe";
}

TEST(OverlayFormatTests, TheLayerTurnsWithTheRoom)
{
	LevelData level = LevelOf(PATCH);
	LevelData turned = RoguelikeGame::TransformRoom(level, 1, false);

	ASSERT_FALSE(turned.overlay.empty()) << "the room turned and left its overlay behind";

	int marks = 0;
	for (const auto& row : turned.overlay)
	{
		for (TileType tile : row)
		{
			marks += tile == TileType::Line ? 1 : 0;
		}
	}

	EXPECT_EQ(marks, 3) << "the overlay lost cells while turning";
}

TEST(OverlayFormatTests, ARoomWithoutALayerTurnsWithoutOne)
{
	LevelData turned = RoguelikeGame::TransformRoom(LevelOf(PLAIN), 1, false);

	EXPECT_TRUE(turned.overlay.empty());
}

TEST(OverlayFrameTests, MarkingsOnTheUpperLayerComeFromTheirOwnRow)
{
	LevelData level = LevelOf(PATCH);

	sf::IntRect frame = RoguelikeGame::OverlayFrameFor(level, 2, 1);

	EXPECT_EQ(frame.top, RoguelikeGame::TILE_OVERLAY_ROW * RoguelikeGame::TILE_FRAME_SIZE)
		<< "the overlay marking dragged the asphalt row along with it";
}

TEST(OverlayFrameTests, TheTwoLayersReadTheirOwnNeighbours)
{
	LevelData level = LevelOf(PATCH);

	// Разметка лежит в ряд, значит штрих поперёк клетки - и наверху, и внизу правило одно.
	EXPECT_EQ(RoguelikeGame::LineFrame(level.overlay, 2, 1), RoguelikeGame::TILE_LINE_ACROSS);
	EXPECT_EQ(RoguelikeGame::LineFrame(level.tiles, 2, 1), RoguelikeGame::TILE_LINE_ACROSS);
}

namespace
{
	class ShippedOverlayTest : public ProjectFiles::Test
	{
	};
}

TEST_F(ShippedOverlayTest, ABridgeSizedLayerIsBuiltInNoTime)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	auto started = std::chrono::steady_clock::now();

	int cells = 0;
	for (int row = 0; row < bridge.height; row++)
	{
		for (int column = 0; column < bridge.width; column++)
		{
			cells += OverlayAt(bridge, column, row) != TileType::Empty ? 1 : 0;
		}
	}

	double spent = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();

	std::cout << "overlay pass over " << bridge.width << "x" << bridge.height
		<< " tiles: " << spent << " ms, " << cells << " cells drawn" << std::endl;

	EXPECT_LT(spent, 50.0) << "walking the layer is too slow for a map this size";
}

TEST_F(ShippedOverlayTest, TheBridgeMarkingsMovedUpstairs)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	int upstairs = 0;
	int downstairs = 0;

	for (int row = 0; row < bridge.height; row++)
	{
		for (int column = 0; column < bridge.width; column++)
		{
			upstairs += OverlayAt(bridge, column, row) == TileType::Line ? 1 : 0;
			downstairs += TileAt(bridge, column, row) == TileType::Line ? 1 : 0;
		}
	}

	EXPECT_GT(upstairs, 0) << "the bridge lost its lane markings";
	EXPECT_EQ(downstairs, 0) << "markings still drag a patch of asphalt along with them";
}

TEST_F(ShippedOverlayTest, UnderEveryMarkingThereIsRoadToDriveOn)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	for (int row = 0; row < bridge.height; row++)
	{
		for (int column = 0; column < bridge.width; column++)
		{
			if (OverlayAt(bridge, column, row) != TileType::Line)
			{
				continue;
			}

			TileType ground = TileAt(bridge, column, row);

			// Накладка ничего не меняет в проходимости, поэтому под ней обязан быть пол.
			EXPECT_TRUE(ground == TileType::Floor || ground == TileType::WaveSpawn)
				<< "a marking at " << column << ";" << row << " floats over nothing to walk on";
		}
	}
}

TEST_F(ShippedOverlayTest, TheMarkingsRunAlongTheBridgeNotAcrossIt)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	int across = 0;
	for (int row = 0; row < bridge.height; row++)
	{
		for (int column = 0; column < bridge.width; column++)
		{
			if (OverlayAt(bridge, column, row) != TileType::Line)
			{
				continue;
			}

			across += RoguelikeGame::LineFrame(bridge.overlay, column, row) == RoguelikeGame::TILE_LINE_ACROSS ? 1 : 0;
		}
	}

	// Разделительная идёт вдоль моста, значит каждый её штрих лежит поперёк своей клетки.
	EXPECT_GT(across, 0) << "the dividing line reads as if it ran across the road";
}

namespace
{
	const char* HOLE =
	"[level]\n"
	"kind hole\n"
	"\n"
	"[legend]\n"
	"# Wall\n"
	". Floor\n"
	"~ Water\n"
	"X Breach\n"
	"@ PlayerSpawn\n"
	"> Exit\n"
	"\n"
	"[map]\n"
	"#######\n"
	"#@...>#\n"
	"#.~~~.#\n"
	"#.~~~.#\n"
	"#.....#\n"
	"#######\n"
	"\n"
	"[overlay]\n"
	"       \n"
	"       \n"
	"  XXX  \n"
	"  XXX  \n"
	"       \n"
	"       \n";
}

TEST(BreachTests, TheEdgeLooksAtTheRoadAroundIt)
{
	LevelData level = LevelOf(HOLE);

	// У верхнего ряда пролома дорога сверху, у нижнего - снизу: маски разные.
	int upper = RoguelikeGame::BreachMask(level, 2, 2);
	int lower = RoguelikeGame::BreachMask(level, 2, 3);

	EXPECT_TRUE(upper & RoguelikeGame::WALL_NEIGHBOUR_UP) << "the edge does not notice the road above it";
	EXPECT_FALSE(upper & RoguelikeGame::WALL_NEIGHBOUR_DOWN) << "the edge invented a road where there is water";
	EXPECT_TRUE(lower & RoguelikeGame::WALL_NEIGHBOUR_DOWN);
	EXPECT_NE(upper, lower) << "both sides of the hole are drawn with the same piece of concrete";
}

TEST(BreachTests, TheEdgeTakesItsFrameFromItsOwnRow)
{
	LevelData level = LevelOf(HOLE);

	sf::IntRect frame = RoguelikeGame::OverlayFrameFor(level, 2, 2);

	EXPECT_EQ(frame.top, RoguelikeGame::TILE_BREACH_ROW * RoguelikeGame::TILE_FRAME_SIZE);
	EXPECT_LT(frame.left, RoguelikeGame::TILE_BREACH_FRAMES * RoguelikeGame::TILE_FRAME_SIZE)
		<< "the frame ran past the end of its row";
}

TEST(BreachTests, AnyMaskLandsInsideTheRow)
{
	for (int mask = -4; mask < 64; mask++)
	{
		int frame = RoguelikeGame::BreachFrame(mask);

		EXPECT_GE(frame, 0) << "mask " << mask;
		EXPECT_LT(frame, RoguelikeGame::TILE_BREACH_FRAMES) << "mask " << mask;
	}
}

TEST(BreachTests, UnderTheEdgeThereIsWaterAndNotThinAir)
{
	LevelData level = LevelOf(HOLE);

	for (int row = 0; row < level.height; row++)
	{
		for (int column = 0; column < level.width; column++)
		{
			if (OverlayAt(level, column, row) != TileType::Breach)
			{
				continue;
			}

			EXPECT_EQ(TileAt(level, column, row), TileType::Water)
				<< "the edge at " << column << ";" << row << " hangs over nothing";
		}
	}
}

TEST_F(ShippedOverlayTest, TheBridgeHasBreachesAndTheyAreDrawn)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	int edges = 0;
	for (int row = 0; row < bridge.height; row++)
	{
		for (int column = 0; column < bridge.width; column++)
		{
			if (OverlayAt(bridge, column, row) != TileType::Breach)
			{
				continue;
			}

			edges++;

			EXPECT_EQ(TileAt(bridge, column, row), TileType::Water)
				<< "a broken edge at " << column << ";" << row << " lies on solid road";
			EXPECT_NE(RoguelikeGame::BreachMask(bridge, column, row), 0)
				<< "a broken edge at " << column << ";" << row << " touches no road at all";
		}
	}

	// Два пролома на мост, у каждого край не меньше восьми клеток.
	EXPECT_GT(edges, 16) << "the bridge is barely broken anywhere";
}

TEST_F(ShippedOverlayTest, TheBreachesAreSpreadAlongTheBridge)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	int leftmost = bridge.width;
	int rightmost = 0;

	for (int row = 0; row < bridge.height; row++)
	{
		for (int column = 0; column < bridge.width; column++)
		{
			if (OverlayAt(bridge, column, row) != TileType::Breach)
			{
				continue;
			}

			leftmost = column < leftmost ? column : leftmost;
			rightmost = column > rightmost ? column : rightmost;
		}
	}

	ASSERT_LT(leftmost, rightmost) << "the bridge has no breaches at all";

	// Проломы должны встречаться по всей длине, а не кучей в одном месте.
	EXPECT_GT(rightmost - leftmost, bridge.width / 2)
		<< "every breach sits within the same stretch of the bridge";
}

TEST_F(ShippedOverlayTest, ABreachNeverCutsTheBridgeInTwo)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	// В каждой колонке обязан остаться хоть один проходимый ряд, иначе мост заперт.
	// Торцы не в счёт: они закрыты отбойником, там ехать некуда по замыслу.
	for (int column = 1; column < bridge.width - 1; column++)
	{
		int passable = 0;
		for (int row = 0; row < bridge.height; row++)
		{
			TileType tile = TileAt(bridge, column, row);
			passable += tile == TileType::Floor || tile == TileType::Line || tile == TileType::WaveSpawn ? 1 : 0;
		}

		EXPECT_GT(passable, 0) << "column " << column << " is water from rail to rail";
	}
}

TEST_F(ShippedOverlayTest, BrokenSpansLeaveConcreteOnTheRoad)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");

	int rubble = 0;
	for (const RoguelikeGame::PropPlacement& prop : bridge.props)
	{
		rubble += prop.propId.rfind("bridge_rubble", 0) == 0 ? 1 : 0;
	}

	// Два обрушения на мосту, на каждое - не меньше четырёх кусков вокруг пролома.
	// Точное число зависит от раскладки секций и сторожить его бессмысленно.
	EXPECT_GE(rubble, 8) << "a collapse that threw no concrete onto the deck";
}
