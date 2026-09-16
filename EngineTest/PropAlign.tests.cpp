#include "pch.h"
#include "LevelLoader.h"
#include "ProjectFiles.h"
#include "PropAlign.h"
#include "PropCatalog.h"
#include <filesystem>
#include <sstream>

using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PanelAngle;
using RoguelikeGame::PanelSupport;
using RoguelikeGame::PANEL_UPRIGHT_ANGLE;
using RoguelikeGame::PropCatalog;
using RoguelikeGame::PropDefinition;
using RoguelikeGame::ReadPanelSupport;

namespace
{
	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "panels");
	}

	const char* SHOP_ROOM =
		"[level]\n"
		"kind shop\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"G Prop:shop_glass\n"
		"\n"
		"[map]\n"
		"#####\n"
		".G..#\n"
		".G..#\n"
		".G..#\n"
		"#####\n";

	const char* COUNTER_ROOM =
		"[level]\n"
		"kind shop\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"G Prop:shop_glass\n"
		"\n"
		"[map]\n"
		"#####\n"
		"..GGG\n"
		".....\n"
		".....\n"
		"#####\n";
}

TEST(PanelAngleTests, SupportAboveAndBelowStandsThePanelUp)
{
	EXPECT_FLOAT_EQ(PanelAngle({true, true, false, false}), PANEL_UPRIGHT_ANGLE);
}

TEST(PanelAngleTests, SupportOnTheSidesLeavesThePanelFlat)
{
	EXPECT_FLOAT_EQ(PanelAngle({false, false, true, true}), 0.f);
}

TEST(PanelAngleTests, ASingleNeighbourAboveIsEnoughToStandUp)
{
	EXPECT_FLOAT_EQ(PanelAngle({true, false, false, false}), PANEL_UPRIGHT_ANGLE);
}

TEST(PanelAngleTests, EqualSupportKeepsThePanelAsDrawn)
{
	EXPECT_FLOAT_EQ(PanelAngle({true, false, true, false}), 0.f);
	EXPECT_FLOAT_EQ(PanelAngle({false, false, false, false}), 0.f);
}

TEST(PanelAngleTests, AColumnOfGlassStandsUpright)
{
	LevelData level = LevelOf(SHOP_ROOM);

	for (int row = 1; row <= 3; row++)
	{
		PanelSupport support = ReadPanelSupport(level, "shop_glass", 1, row);

		EXPECT_FLOAT_EQ(PanelAngle(support), PANEL_UPRIGHT_ANGLE) << "row " << row;
	}
}

TEST(PanelAngleTests, ARowOfGlassStaysFlat)
{
	LevelData level = LevelOf(COUNTER_ROOM);

	for (int column = 2; column <= 4; column++)
	{
		PanelSupport support = ReadPanelSupport(level, "shop_glass", column, 1);

		EXPECT_FLOAT_EQ(PanelAngle(support), 0.f) << "column " << column;
	}
}

namespace
{
	class ShippedPanelsTest : public ProjectFiles::Test
	{
	};
}

TEST_F(ShippedPanelsTest, TheGlassInTheShippedRoomsStandsAlongTheWall)
{
	ASSERT_TRUE(isFound) << "Resources not found from " << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");
	const PropDefinition* glass = props.Find("shop_glass");
	ASSERT_NE(glass, nullptr);
	EXPECT_TRUE(glass->isPanel) << "shop_glass is not marked as a panel";

	int checked = 0;
	for (const auto& entry : std::filesystem::directory_iterator("Resources/Rooms"))
	{
		LevelData room = LevelLoader::Load(entry.path().string());

		for (const RoguelikeGame::PropPlacement& placement : room.props)
		{
			if (placement.propId != "shop_glass")
			{
				continue;
			}

			PanelSupport support = ReadPanelSupport(room, placement.propId, placement.column, placement.row);

			EXPECT_FLOAT_EQ(PanelAngle(support), PANEL_UPRIGHT_ANGLE)
				<< entry.path().filename().string() << " glass at " << placement.column << ";" << placement.row;
			checked++;
		}
	}

	EXPECT_GT(checked, 0) << "no glass found in the shipped rooms";
}
