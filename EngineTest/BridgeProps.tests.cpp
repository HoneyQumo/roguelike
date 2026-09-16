#include "pch.h"
#include "ProjectFiles.h"
#include "GameSettings.h"
#include "PropCatalog.h"
#include <SFML/Graphics/Image.hpp>
#include <sstream>

using RoguelikeGame::PropCatalog;
using RoguelikeGame::PropDefinition;

namespace
{
	PropCatalog CatalogOf(const std::string& text)
	{
		std::istringstream input(text);

		return PropCatalog::Parse(input, "bridge");
	}

	class ShippedBridgePropsTest : public ProjectFiles::Test
	{
	};

	// Всё, что стоит на полотне моста.
	const char* BRIDGE_PROPS[] = {
		"car_sedan", "car_van", "car_small", "car_wreck",
		"road_barrier", "road_cone", "fuel_barrel", "tire_stack",
		"oil_slick", "skid_mark"
	};
}

TEST(PropSizeTests, APropWithoutItsOwnHeightStaysSquare)
{
	PropCatalog catalog = CatalogOf(
		"[prop crate]\n"
		"name Crate\n"
		"size 48\n");

	const PropDefinition* crate = catalog.Find("crate");
	ASSERT_NE(crate, nullptr);

	EXPECT_FLOAT_EQ(crate->Height(), 48.f);
}

TEST(PropSizeTests, APropCanBeLongerThanItIsWide)
{
	PropCatalog catalog = CatalogOf(
		"[prop car]\n"
		"name Car\n"
		"size 118\n"
		"height 62\n");

	const PropDefinition* car = catalog.Find("car");
	ASSERT_NE(car, nullptr);

	EXPECT_FLOAT_EQ(car->size, 118.f);
	EXPECT_FLOAT_EQ(car->Height(), 62.f);
}

TEST_F(ShippedBridgePropsTest, TheBridgeHasEverythingItNeedsToStandOn)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : BRIDGE_PROPS)
	{
		const PropDefinition* prop = props.Find(id);

		ASSERT_NE(prop, nullptr) << id << " is missing from the catalog";
		EXPECT_TRUE(prop->HasFrame()) << id << " has no sprite";
		EXPECT_GT(prop->size, 0.f) << id;
	}
}

TEST_F(ShippedBridgePropsTest, CarsAreWideCoverAndLitterIsNot)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : {"car_sedan", "car_van", "car_small", "car_wreck"})
	{
		const PropDefinition* car = props.Find(id);
		ASSERT_NE(car, nullptr) << id;

		EXPECT_TRUE(car->isCover) << id << " is not cover";
		EXPECT_TRUE(car->isSolid) << id << " lets bullets and bodies through";
		EXPECT_GT(car->size, car->Height()) << id << " does not lie along the road";
	}

	for (const char* id : {"oil_slick", "skid_mark", "road_cone"})
	{
		const PropDefinition* litter = props.Find(id);
		ASSERT_NE(litter, nullptr) << id;

		EXPECT_FALSE(litter->isSolid) << id << " blocks the road";
		EXPECT_FALSE(litter->isCover) << id << " pretends to be cover";
	}
}

TEST_F(ShippedBridgePropsTest, TheBridgeAtlasHoldsEveryFrameItPromises)
{
	ASSERT_TRUE(isFound) << previous.string();

	sf::Image atlas;
	ASSERT_TRUE(atlas.loadFromFile("Resources/Textures/props_bridge.png"));

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : BRIDGE_PROPS)
	{
		const PropDefinition* prop = props.Find(id);
		ASSERT_NE(prop, nullptr) << id;

		EXPECT_GE(prop->frame.left, 0) << id;
		EXPECT_LE(prop->frame.left + prop->frame.width, static_cast<int>(atlas.getSize().x)) << id << " reads past the atlas";
		EXPECT_LE(prop->frame.top + prop->frame.height, static_cast<int>(atlas.getSize().y)) << id << " reads past the atlas";
	}
}

TEST_F(ShippedBridgePropsTest, EveryFrameHasTheShapeOfItsBox)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : BRIDGE_PROPS)
	{
		const PropDefinition* prop = props.Find(id);
		ASSERT_NE(prop, nullptr) << id;
		ASSERT_GT(prop->frame.height, 0) << id;

		float frameShape = static_cast<float>(prop->frame.width) / static_cast<float>(prop->frame.height);
		float boxShape = prop->size / prop->Height();

		// Кадр, растянутый в бокс другой формы, сплющивает рисунок.
		EXPECT_NEAR(frameShape, boxShape, 0.12f) << id << " is stretched: frame " << frameShape << " box " << boxShape;
	}
}

TEST_F(ShippedBridgePropsTest, CarsAreAboutTwoTilesLongAndOneWide)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : {"car_sedan", "car_van", "car_small", "car_wreck"})
	{
		const PropDefinition* car = props.Find(id);
		ASSERT_NE(car, nullptr) << id;

		EXPECT_GE(car->size, 1.7f * RoguelikeGame::TILE_SIZE) << id << " is too short for a car";
		EXPECT_LE(car->size, 2.2f * RoguelikeGame::TILE_SIZE) << id << " is longer than two tiles";
		EXPECT_GE(car->Height(), 0.8f * RoguelikeGame::TILE_SIZE) << id << " is too narrow";
		EXPECT_LE(car->Height(), 1.1f * RoguelikeGame::TILE_SIZE) << id << " is wider than a lane";
	}
}

TEST_F(ShippedBridgePropsTest, NoCarLooksLikeAToyNextToTheOthers)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	float smallest = 0.f;
	float largest = 0.f;

	for (const char* id : {"car_sedan", "car_van", "car_small", "car_wreck"})
	{
		const PropDefinition* car = props.Find(id);
		ASSERT_NE(car, nullptr) << id;

		smallest = smallest == 0.f || car->size < smallest ? car->size : smallest;
		largest = car->size > largest ? car->size : largest;
	}

	EXPECT_GE(smallest, 0.85f * largest) << "the smallest car reads as a toy beside the biggest";
}

TEST_F(ShippedBridgePropsTest, EveryCarKeepsTheShapeOfItsOwnFrame)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : {"car_sedan", "car_van", "car_small", "car_wreck"})
	{
		const PropDefinition* car = props.Find(id);
		ASSERT_NE(car, nullptr) << id;
		ASSERT_GT(car->frame.height, 0) << id;

		float frameShape = static_cast<float>(car->frame.width) / static_cast<float>(car->frame.height);
		float boxShape = car->size / car->Height();

		EXPECT_NEAR(boxShape, frameShape, 0.05f) << id << " is squashed against its own sprite";
	}
}

TEST_F(ShippedBridgePropsTest, ABurntOutCarStaysInTheWay)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : {"car_sedan", "car_van", "car_small"})
	{
		const PropDefinition* car = props.Find(id);
		ASSERT_NE(car, nullptr) << id;

		EXPECT_TRUE(car->leavesWreck) << id << " can be walked through once it burns out";
	}

	const PropDefinition* barrel = props.Find("fuel_barrel");
	ASSERT_NE(barrel, nullptr);
	EXPECT_FALSE(barrel->leavesWreck) << "a burst barrel should not block the road";
}
