#include "pch.h"
#include "ProjectFiles.h"
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
