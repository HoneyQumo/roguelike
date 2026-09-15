#include "pch.h"
#include "PropCatalog.h"
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
