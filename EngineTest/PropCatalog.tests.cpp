#include "pch.h"
#include "PropCatalog.h"
#include "ProjectFiles.h"
#include <SFML/Graphics/Image.hpp>
#include <sstream>

using RoguelikeGame::PropCatalog;
using RoguelikeGame::PropDefinition;

namespace
{
	PropCatalog ParseProps(const std::string& text)
	{
		std::istringstream input(text);

		return PropCatalog::Parse(input, "props.config");
	}
}

TEST(PropCatalogTest, PropIsParsedWithAllItsFields)
{
	PropCatalog catalog = ParseProps(
		"; ящик\n"
		"[prop crate_wood]\n"
		"name Ящик\n"
		"health 40\n"
		"size 48\n"
		"color 150 110 60\n"
		"brokenColor 80 60 35\n"
		"loot crate_wood\n");

	const PropDefinition* prop = catalog.Find("crate_wood");

	ASSERT_NE(prop, nullptr);
	EXPECT_EQ(prop->name, "Ящик");
	EXPECT_FLOAT_EQ(prop->health, 40.f);
	EXPECT_FLOAT_EQ(prop->size, 48.f);
	EXPECT_EQ(prop->lootTable, "crate_wood");
	EXPECT_EQ(prop->color, sf::Color(150, 110, 60));
	EXPECT_EQ(prop->brokenColor, sf::Color(80, 60, 35));
	EXPECT_TRUE(prop->IsDestructible());
}

TEST(PropCatalogTest, PropWithoutHealthIsNotDestructible)
{
	PropCatalog catalog = ParseProps(
		"[prop pillar]\n"
		"name Колонна\n"
		"size 64\n");

	const PropDefinition* prop = catalog.Find("pillar");

	ASSERT_NE(prop, nullptr);
	EXPECT_FALSE(prop->IsDestructible());
}

TEST(PropCatalogTest, PropWithoutLootBreaksWithoutReward)
{
	PropCatalog catalog = ParseProps(
		"[prop crate_empty]\n"
		"health 30\n");

	const PropDefinition* prop = catalog.Find("crate_empty");

	ASSERT_NE(prop, nullptr);
	EXPECT_TRUE(prop->IsDestructible());
	EXPECT_TRUE(prop->lootTable.empty());
}

TEST(PropCatalogTest, SeveralPropsLiveInOneFile)
{
	PropCatalog catalog = ParseProps(
		"[prop crate_wood]\n"
		"health 40\n"
		"[prop barrel_rusty]\n"
		"health 25\n");

	EXPECT_EQ(catalog.Size(), 2u);
	EXPECT_NE(catalog.Find("barrel_rusty"), nullptr);
	EXPECT_EQ(catalog.Find("statue"), nullptr);
}

TEST(PropCatalogTest, BrokenLinesAreReported)
{
	EXPECT_THROW(ParseProps("health 10\n"), std::runtime_error);
	EXPECT_THROW(ParseProps("[prop ]\n"), std::runtime_error);
	EXPECT_THROW(ParseProps("[prop crate]\nhealth -5\n"), std::runtime_error);
	EXPECT_THROW(ParseProps("[prop crate]\nsize 0\n"), std::runtime_error);
	EXPECT_THROW(ParseProps("[prop crate]\ncolor 150 110\n"), std::runtime_error);
}

TEST(PropCatalogTest, UnknownFieldIsSkipped)
{
	PropCatalog catalog = ParseProps(
		"[prop crate_wood]\n"
		"weight heavy\n"
		"health 40\n");

	EXPECT_FLOAT_EQ(catalog.Find("crate_wood")->health, 40.f);
}

TEST(PropCatalogTest, EmptyCatalogIsShared)
{
	EXPECT_TRUE(PropCatalog::Empty().IsEmpty());
	EXPECT_EQ(PropCatalog::Empty().Find("crate_wood"), nullptr);
}

TEST(PropCatalogTest, HitEffectComesFromTheData)
{
	PropCatalog catalog = ParseProps(
		"[prop crate_wood]\n"
		"health 40\n"
		"hit impact\n"
		"[prop flesh_pile]\n"
		"health 10\n"
		"hit blood\n"
		"[prop statue]\n"
		"health 90\n");

	EXPECT_EQ(catalog.Find("crate_wood")->hitEffect, "impact");
	EXPECT_EQ(catalog.Find("flesh_pile")->hitEffect, "blood");
	EXPECT_EQ(catalog.Find("statue")->hitEffect, "impact");
}

TEST(PropCatalogTest, APropIsSolidAndSeeThroughByDefault)
{
	PropCatalog catalog = ParseProps("[prop crate]\nname Crate\n");

	const PropDefinition* prop = catalog.Find("crate");
	ASSERT_NE(prop, nullptr);
	EXPECT_TRUE(prop->isSolid);
	EXPECT_FALSE(prop->isCover);
}

TEST(PropCatalogTest, AShelfIsSolidAndBlocksTheView)
{
	PropCatalog catalog = ParseProps("[prop shelf]\nname Shelf\ncover true\n");

	const PropDefinition* prop = catalog.Find("shelf");
	ASSERT_NE(prop, nullptr);
	EXPECT_TRUE(prop->isSolid);
	EXPECT_TRUE(prop->isCover);
}

TEST(PropCatalogTest, ABushIsWalkedThroughAndBlocksTheView)
{
	PropCatalog catalog = ParseProps("[prop bush]\nname Bush\nsolid false\ncover true\n");

	const PropDefinition* prop = catalog.Find("bush");
	ASSERT_NE(prop, nullptr);
	EXPECT_FALSE(prop->isSolid);
	EXPECT_TRUE(prop->isCover);
}

namespace
{
	class ShippedPropsTest : public ProjectFiles::Test
	{
	protected:
		PropCatalog Shipped()
		{
			return PropCatalog::Load("Resources/Props/props.config");
		}
	};
}

TEST_F(ShippedPropsTest, EveryFrameFitsInsideItsAtlas)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog catalog = Shipped();
	ASSERT_FALSE(catalog.IsEmpty());

	for (const PropDefinition& prop : catalog)
	{
		if (!prop.HasFrame())
		{
			continue;
		}

		sf::Image atlas;
		ASSERT_TRUE(atlas.loadFromFile(prop.texturePath)) << prop.id;

		for (const sf::IntRect& frame : {prop.frame, prop.spentFrame})
		{
			if (frame.width == 0 && frame.height == 0)
			{
				continue;
			}

			EXPECT_GE(frame.left, 0) << prop.id;
			EXPECT_GE(frame.top, 0) << prop.id;
			EXPECT_LE(frame.left + frame.width, static_cast<int>(atlas.getSize().x)) << prop.id;
			EXPECT_LE(frame.top + frame.height, static_cast<int>(atlas.getSize().y)) << prop.id;
		}
	}
}

TEST_F(ShippedPropsTest, ABushIsWalkedThroughAndHides)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog catalog = Shipped();
	const PropDefinition* bush = catalog.Find("bush");

	ASSERT_NE(bush, nullptr);
	EXPECT_FALSE(bush->isSolid);
	EXPECT_TRUE(bush->isCover);
}

TEST_F(ShippedPropsTest, GlassAndBarsStopTheWayButNotTheView)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog catalog = Shipped();

	for (const char* id : {"shop_glass", "cell_bars"})
	{
		const PropDefinition* prop = catalog.Find(id);
		ASSERT_NE(prop, nullptr) << id;
		EXPECT_TRUE(prop->isSolid) << id;
		EXPECT_FALSE(prop->isCover) << id;
	}
}

TEST_F(ShippedPropsTest, TallThingsHideWhoeverIsBehindThem)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog catalog = Shipped();

	for (const char* id : {"locker", "bin", "concrete_block"})
	{
		const PropDefinition* prop = catalog.Find(id);
		ASSERT_NE(prop, nullptr) << id;
		EXPECT_TRUE(prop->isSolid) << id;
		EXPECT_TRUE(prop->isCover) << id;
	}
}

TEST_F(ShippedPropsTest, OldCratesStayAsTheyWere)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog catalog = Shipped();
	const PropDefinition* crate = catalog.Find("crate_ammo");

	ASSERT_NE(crate, nullptr);
	EXPECT_TRUE(crate->isSolid);
	EXPECT_FALSE(crate->isCover);
}
