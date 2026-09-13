#include "pch.h"
#include "ItemCatalogLoader.h"
#include "GameSettings.h"
#include <sstream>

using RoguelikeGame::ItemCatalog;
using RoguelikeGame::ItemCatalogLoader;
using RoguelikeGame::ItemEffectKind;
using RoguelikeGame::ItemType;

namespace
{
	ItemCatalog ParseText(const std::string& text)
	{
		std::istringstream input(text);
		return ItemCatalogLoader::Parse(input, "test");
	}

	const std::string POTION =
		"[item potion_small]\n"
		"name Aptechka\n"
		"type Consumable\n"
		"icon Resources/Textures/fx.png 0 224 24 24\n"
		"stackable true\n"
		"maxStack 5\n"
		"effect Heal 35\n";
}

TEST(ItemCatalogTests, ParsesEveryFieldOfAnItem)
{
	ItemCatalog catalog = ParseText(POTION);

	ASSERT_EQ(catalog.Size(), 1u);
	const RoguelikeGame::ItemDefinition* item = catalog.Find("potion_small");
	ASSERT_NE(item, nullptr);

	EXPECT_EQ(item->name, "Aptechka");
	EXPECT_EQ(item->type, ItemType::Consumable);
	EXPECT_EQ(item->icon.texturePath, "Resources/Textures/fx.png");
	EXPECT_EQ(item->icon.rect.left, 0);
	EXPECT_EQ(item->icon.rect.top, 224);
	EXPECT_EQ(item->icon.rect.width, 24);
	EXPECT_TRUE(item->stackable);
	EXPECT_EQ(item->maxStack, 5);
	EXPECT_EQ(item->effect.kind, ItemEffectKind::Heal);
	EXPECT_FLOAT_EQ(item->effect.amount, 35.f);
}

TEST(ItemCatalogTests, ParsesThreeItemsOfDifferentTypes)
{
	ItemCatalog catalog = ParseText(POTION +
		"\n[item key_rusty]\n"
		"name Klyuch\n"
		"type Key\n"
		"icon Resources/Textures/items.png 0 0 16 16\n"
		"effect Unlock 0 door_exit\n"
		"\n[item weapon_deagle]\n"
		"name Deagle\n"
		"type Weapon\n"
		"icon Resources/Textures/weapons_side.png 0 192 112 32\n"
		"effect EquipWeapon 0 Deagle\n");

	EXPECT_EQ(catalog.Size(), 3u);
	EXPECT_EQ(catalog.Find("key_rusty")->type, ItemType::Key);
	EXPECT_EQ(catalog.Find("key_rusty")->effect.target, "door_exit");
	EXPECT_EQ(catalog.Find("weapon_deagle")->type, ItemType::Weapon);
	EXPECT_EQ(catalog.Find("weapon_deagle")->effect.kind, ItemEffectKind::EquipWeapon);
}

TEST(ItemCatalogTests, UnknownIdIsNotFound)
{
	ItemCatalog catalog = ParseText(POTION);

	EXPECT_EQ(catalog.Find("no_such_item"), nullptr);
	EXPECT_FALSE(catalog.Contains("no_such_item"));
}

TEST(ItemCatalogTests, DuplicateIdIsRejected)
{
	EXPECT_THROW(ParseText(POTION + "\n" + POTION), std::runtime_error);
}

TEST(ItemCatalogTests, UnknownTypeIsRejected)
{
	EXPECT_THROW(ParseText(
		"[item broken]\n"
		"name Broken\n"
		"type NoSuchType\n"
		"icon a.png 0 0 8 8\n"), std::runtime_error);
}

TEST(ItemCatalogTests, UnknownEffectIsRejected)
{
	EXPECT_THROW(ParseText(
		"[item broken]\n"
		"name Broken\n"
		"type Consumable\n"
		"icon a.png 0 0 8 8\n"
		"effect Teleport 1\n"), std::runtime_error);
}

TEST(ItemCatalogTests, ItemWithoutNameOrIconIsRejected)
{
	EXPECT_THROW(ParseText("[item no_name]\ntype Key\nicon a.png 0 0 8 8\n"), std::runtime_error);
	EXPECT_THROW(ParseText("[item no_icon]\nname Something\ntype Key\n"), std::runtime_error);
}

TEST(ItemCatalogTests, BrokenIconRectIsRejected)
{
	EXPECT_THROW(ParseText("[item bad]\nname Bad\ntype Key\nicon a.png 0 0 8\n"), std::runtime_error);
	EXPECT_THROW(ParseText("[item bad]\nname Bad\ntype Key\nicon a.png 0 0 0 8\n"), std::runtime_error);
}

TEST(ItemCatalogTests, CommentsAndBlankLinesAreSkipped)
{
	ItemCatalog catalog = ParseText(
		"; comment line\n"
		"\n"
		"[item potion_small]\n"
		"; another comment\n"
		"name Aptechka\n"
		"type Consumable\n"
		"icon a.png 0 0 8 8\n"
		"\n");

	EXPECT_EQ(catalog.Size(), 1u);
}

TEST(ItemCatalogTests, UnknownFieldDoesNotBreakTheItem)
{
	ItemCatalog catalog = ParseText(
		"[item potion_small]\n"
		"name Aptechka\n"
		"type Consumable\n"
		"icon a.png 0 0 8 8\n"
		"futureField 42\n");

	ASSERT_EQ(catalog.Size(), 1u);
	EXPECT_EQ(catalog.Find("potion_small")->name, "Aptechka");
}

TEST(ItemCatalogTests, EmptyInputGivesEmptyCatalog)
{
	EXPECT_EQ(ParseText("").Size(), 0u);
	EXPECT_EQ(ItemCatalog::Empty().Size(), 0u);
}

TEST(ItemCatalogTests, FieldOutsideOfBlockIsRejected)
{
	EXPECT_THROW(ParseText("name Lost\n[item x]\nname X\ntype Key\nicon a.png 0 0 8 8\n"), std::runtime_error);
}

TEST(ItemCatalogTest, AmmoEffectIsParsed)
{
	std::istringstream input(
		"[item ammo_pistol]\n"
		"name Pistol ammo\n"
		"type Consumable\n"
		"icon Resources/Textures/fx.png 0 288 24 8\n"
		"stackable true\n"
		"maxStack 8\n"
		"effect AddAmmo 24 pistol\n");

	RoguelikeGame::ItemCatalog catalog = RoguelikeGame::ItemCatalogLoader::Parse(input, "items.config");
	const RoguelikeGame::ItemDefinition* item = catalog.Find("ammo_pistol");

	ASSERT_NE(item, nullptr);
	EXPECT_EQ(item->effect.kind, RoguelikeGame::ItemEffectKind::AddAmmo);
	EXPECT_FLOAT_EQ(item->effect.amount, 24.f);
	EXPECT_EQ(item->effect.target, "pistol");
}

TEST(ItemCatalogTest, IconScaleIsATuningNotASizeSource)
{
	std::istringstream input(
		"[item weapon_glock]\n"
		"name Glock\n"
		"type Weapon\n"
		"icon Resources/Textures/weapons_side.png 0 160 112 32\n"
		"scale 1\n"
		"[item ammo_pistol]\n"
		"name Ammo\n"
		"type Consumable\n"
		"icon Resources/Textures/fx.png 0 288 24 8\n"
		"scale 0.9\n");

	RoguelikeGame::ItemCatalog catalog = RoguelikeGame::ItemCatalogLoader::Parse(input, "items.config");

	const RoguelikeGame::ItemDefinition* gun = catalog.Find("weapon_glock");
	const RoguelikeGame::ItemDefinition* ammo = catalog.Find("ammo_pistol");

	ASSERT_NE(gun, nullptr);
	ASSERT_NE(ammo, nullptr);

	float gunSize = RoguelikeGame::ITEM_WORLD_SIZE * gun->icon.worldScale;
	float ammoSize = RoguelikeGame::ITEM_WORLD_SIZE * ammo->icon.worldScale;

	EXPECT_NEAR(gunSize, ammoSize, RoguelikeGame::ITEM_WORLD_SIZE * 0.25f);
}
