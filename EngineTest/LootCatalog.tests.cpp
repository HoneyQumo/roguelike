#include "pch.h"
#include "LootCatalog.h"
#include <sstream>

using RoguelikeGame::LootCatalog;
using RoguelikeGame::LootDrop;
using RoguelikeGame::LootTable;

namespace
{
	LootCatalog ParseLoot(const std::string& text)
	{
		std::istringstream input(text);

		return LootCatalog::Parse(input, "loot.config");
	}
}

TEST(LootCatalogTest, TableIsParsedWithItsEntries)
{
	LootCatalog catalog = ParseLoot(
		"; таблица мародёра\n"
		"[table marauder]\n"
		"drop weapon_glock 1 1\n"
		"drop ammo_pistol 3 12\n"
		"nothing 2\n");

	const LootTable* table = catalog.Find("marauder");

	ASSERT_NE(table, nullptr);
	ASSERT_EQ(table->entries.size(), 2u);
	EXPECT_EQ(table->entries[0].itemId, "weapon_glock");
	EXPECT_EQ(table->entries[1].weight, 3);
	EXPECT_EQ(table->entries[1].count, 12);
	EXPECT_EQ(table->emptyWeight, 2);
	EXPECT_EQ(table->rolls, 1);
	EXPECT_EQ(table->GetTotalWeight(), 6);
}

TEST(LootCatalogTest, SeveralTablesLiveInOneFile)
{
	LootCatalog catalog = ParseLoot(
		"[table grunt]\n"
		"drop ammo_pistol\n"
		"[table boss]\n"
		"rolls 3\n"
		"drop potion_small 2 1\n");

	EXPECT_EQ(catalog.Size(), 2u);
	EXPECT_NE(catalog.Find("grunt"), nullptr);
	EXPECT_EQ(catalog.Find("boss")->rolls, 3);
	EXPECT_EQ(catalog.Find("sniper"), nullptr);
}

TEST(LootCatalogTest, MissingWeightAndCountDefaultToOne)
{
	LootCatalog catalog = ParseLoot(
		"[table grunt]\n"
		"drop ammo_pistol\n");

	const LootTable* table = catalog.Find("grunt");

	ASSERT_NE(table, nullptr);
	EXPECT_EQ(table->entries[0].weight, 1);
	EXPECT_EQ(table->entries[0].count, 1);
}

TEST(LootCatalogTest, PickFollowsTheWeights)
{
	LootCatalog catalog = ParseLoot(
		"[table grunt]\n"
		"drop ammo_pistol 3 1\n"
		"drop potion_small 1 1\n"
		"nothing 2\n");

	const LootTable* table = catalog.Find("grunt");
	ASSERT_NE(table, nullptr);
	ASSERT_EQ(table->GetTotalWeight(), 6);

	EXPECT_EQ(table->Pick(0).itemId, "ammo_pistol");
	EXPECT_EQ(table->Pick(2).itemId, "ammo_pistol");
	EXPECT_EQ(table->Pick(3).itemId, "potion_small");
	EXPECT_TRUE(table->Pick(4).IsEmpty());
	EXPECT_TRUE(table->Pick(5).IsEmpty());
}

TEST(LootCatalogTest, PickIsSafeOutsideTheRange)
{
	LootCatalog catalog = ParseLoot(
		"[table grunt]\n"
		"drop ammo_pistol 2 1\n");

	const LootTable* table = catalog.Find("grunt");

	EXPECT_EQ(table->Pick(-5).itemId, "ammo_pistol");
	EXPECT_EQ(table->Pick(1000).itemId, "ammo_pistol");
}

TEST(LootCatalogTest, EmptyTableDropsNothing)
{
	LootCatalog catalog = ParseLoot("[table empty]\n");

	const LootTable* table = catalog.Find("empty");

	ASSERT_NE(table, nullptr);
	EXPECT_EQ(table->GetTotalWeight(), 0);
	EXPECT_TRUE(table->Pick(0).IsEmpty());
}

TEST(LootCatalogTest, BrokenLinesAreReported)
{
	EXPECT_THROW(ParseLoot("drop ammo_pistol 1 1\n"), std::runtime_error);
	EXPECT_THROW(ParseLoot("[table ]\n"), std::runtime_error);
	EXPECT_THROW(ParseLoot("[table grunt]\ndrop\n"), std::runtime_error);
	EXPECT_THROW(ParseLoot("[table grunt]\ndrop ammo_pistol 0 1\n"), std::runtime_error);
	EXPECT_THROW(ParseLoot("[table grunt]\nrolls 0\n"), std::runtime_error);
}

TEST(LootCatalogTest, UnknownFieldIsSkipped)
{
	LootCatalog catalog = ParseLoot(
		"[table grunt]\n"
		"rarity legendary\n"
		"drop ammo_pistol 1 1\n");

	EXPECT_EQ(catalog.Find("grunt")->entries.size(), 1u);
}

TEST(LootCatalogTest, EmptyCatalogIsShared)
{
	EXPECT_TRUE(LootCatalog::Empty().IsEmpty());
	EXPECT_EQ(LootCatalog::Empty().Find("grunt"), nullptr);
}
