#include "pch.h"
#include "ActAssembler.h"
#include "LevelLoader.h"
#include "ProjectFiles.h"
#include "PropAlign.h"
#include "PropCatalog.h"
#include "RoomTransform.h"
#include <sstream>

using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::PropAngle;
using RoguelikeGame::PropCatalog;
using RoguelikeGame::PropDefinition;
using RoguelikeGame::PropPlacement;
using RoguelikeGame::TransformRoom;

namespace
{
	LevelData LevelOf(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "angles");
	}

	const char* CRASH_ROOM =
		"[level]\n"
		"kind crash\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"c Prop:car_sedan\n"
		"C Prop:car_sedan@72\n"
		"R Prop:bridge_rubble\n"
		"\n"
		"[map]\n"
		"#####\n"
		"#c.C#\n"
		"#.R.#\n"
		"#####\n";

	const PropPlacement* Find(const LevelData& level, int column, int row)
	{
		for (const PropPlacement& prop : level.props)
		{
			if (prop.column == column && prop.row == row)
			{
				return &prop;
			}
		}

		return nullptr;
	}

	class ShippedAnglesTest : public ProjectFiles::Test
	{
	};
}

TEST(PropAngleTests, APropWithoutAnAngleStandsStraight)
{
	LevelData level = LevelOf(CRASH_ROOM);

	const PropPlacement* car = Find(level, 1, 1);
	ASSERT_NE(car, nullptr);

	EXPECT_FLOAT_EQ(car->angle, 0.f);
	EXPECT_EQ(car->propId, "car_sedan");
}

TEST(PropAngleTests, TheAngleIsReadAfterTheAtSign)
{
	LevelData level = LevelOf(CRASH_ROOM);

	const PropPlacement* turned = Find(level, 3, 1);
	ASSERT_NE(turned, nullptr);

	EXPECT_FLOAT_EQ(turned->angle, 72.f);
	EXPECT_EQ(turned->propId, "car_sedan") << "the angle leaked into the prop id";
}

TEST(PropAngleTests, TurningTheRoomTurnsWhatStandsInIt)
{
	LevelData level = LevelOf(CRASH_ROOM);
	LevelData turned = TransformRoom(level, 1, false);

	bool isFound = false;
	for (const PropPlacement& prop : turned.props)
	{
		if (prop.propId == "car_sedan" && prop.angle > 0.f)
		{
			isFound = true;
			EXPECT_GE(prop.angle, 90.f) << "the room turned but the prop did not";
		}
	}

	EXPECT_TRUE(isFound);
}

TEST(PropAngleTests, ABadAngleIsRefused)
{
	EXPECT_THROW(LevelOf(
		"[level]\n"
		"kind bad\n"
		"\n"
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"c Prop:car_sedan@turn\n"
		"\n"
		"[map]\n"
		"###\n"
		"#c#\n"
		"###\n"), std::runtime_error);
}

TEST_F(ShippedAnglesTest, TheSameSpotAlwaysGivesTheSameLean)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");
	const PropDefinition* car = props.Find("car_sedan");
	ASSERT_NE(car, nullptr);
	ASSERT_GT(car->jitterDegrees, 0.f) << "cars stand in a line";

	PropPlacement here{7, 9, "car_sedan", 0.f};
	PropPlacement there{8, 9, "car_sedan", 0.f};

	EXPECT_FLOAT_EQ(PropAngle(*car, here), PropAngle(*car, here)) << "the lean is not stable";
	EXPECT_NE(PropAngle(*car, here), PropAngle(*car, there)) << "every car leans the same way";
	EXPECT_LE(std::abs(PropAngle(*car, here)), car->jitterDegrees);
}

TEST_F(ShippedAnglesTest, TheAngleFromTheMapIsKeptOnTopOfTheLean)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");
	const PropDefinition* car = props.Find("car_sedan");
	ASSERT_NE(car, nullptr);

	PropPlacement across{4, 4, "car_sedan", 72.f};

	EXPECT_GT(PropAngle(*car, across), 72.f - car->jitterDegrees);
	EXPECT_LT(PropAngle(*car, across), 72.f + car->jitterDegrees);
}

TEST_F(ShippedAnglesTest, TheBridgeHasRubbleAlongItsBrokenSpans)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : {"bridge_rubble", "bridge_rubble_wide"})
	{
		const PropDefinition* rubble = props.Find(id);

		ASSERT_NE(rubble, nullptr) << id;
		EXPECT_TRUE(rubble->HasFrame()) << id;
		EXPECT_FALSE(rubble->isSolid) << id << " blocks the way round the hole";
	}

	LevelData bridge = RoguelikeGame::LoadAct("Resources/Acts/act1_bridge.config");
	int rubble = 0;
	for (const PropPlacement& prop : bridge.props)
	{
		rubble += prop.propId.rfind("bridge_rubble", 0) == 0 ? 1 : 0;
	}

	// Два обрушения на мосту, на каждое - не меньше четырёх кусков вокруг пролома.
	// Точное число зависит от раскладки секций и сторожить его бессмысленно.
	EXPECT_GE(rubble, 8) << "broken spans have no debris around them";
}
