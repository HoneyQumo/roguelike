#include "pch.h"
#include "ItemCatalogLoader.h"
#include "ProjectFiles.h"
#include "WeaponCatalog.h"
#include <sstream>

using RoguelikeGame::FindWeaponItem;
using RoguelikeGame::ItemCatalog;
using RoguelikeGame::ItemCatalogLoader;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemEffectKind;
using RoguelikeGame::WEAPON_COUNT;
using RoguelikeGame::WEAPONS;

namespace
{
	constexpr int SIDE_FRAME_WIDTH = 112;
	constexpr int SIDE_FRAME_HEIGHT = 32;

	int CountWeaponItems(const ItemCatalog& catalog, const std::string& weaponId)
	{
		int total = 0;

		for (const ItemDefinition& item : catalog)
		{
			total += item.effect.kind == ItemEffectKind::EquipWeapon && item.effect.target == weaponId ? 1 : 0;
		}

		return total;
	}

	class ShippedWeaponItemsTest : public ProjectFiles::Test
	{
	protected:
		ItemCatalog Items() const
		{
			return ItemCatalogLoader::Load("Resources/Items/items.config");
		}
	};
}

TEST(WeaponItemsTest, AWeaponIsFoundByTheNameItsEffectCarries)
{
	std::istringstream input(
		"[item weapon_deagle]\n"
		"name Deagle\n"
		"type Weapon\n"
		"icon Resources/Textures/weapons_side.png 0 192 112 32\n"
		"effect EquipWeapon 0 deagle\n"
		"\n"
		"[item potion_small]\n"
		"name Potion\n"
		"icon Resources/Textures/fx.png 0 224 24 24\n"
		"effect Heal 35\n");

	ItemCatalog catalog = ItemCatalogLoader::Parse(input, "test");

	const ItemDefinition* found = FindWeaponItem(catalog, "deagle");

	ASSERT_NE(found, nullptr);
	EXPECT_EQ(found->id, "weapon_deagle");

	EXPECT_EQ(FindWeaponItem(catalog, "railgun"), nullptr);
	EXPECT_EQ(FindWeaponItem(catalog, "potion_small"), nullptr) << "лечебный эффект принят за оружие";
}

TEST_F(ShippedWeaponItemsTest, EveryWeaponThePlayerCanHoldHasAnItemOfItsOwn)
{
	ASSERT_TRUE(isFound) << "Resources not found from " << previous.string();

	ItemCatalog items = Items();
	ASSERT_GT(items.Size(), 0u);

	for (int index = 0; index < WEAPON_COUNT; index++)
	{
		const std::string weaponId = WEAPONS[index].id;

		EXPECT_EQ(CountWeaponItems(items, weaponId), 1)
			<< "вернуть " << weaponId << " в сумку будет нечем";
	}
}

TEST_F(ShippedWeaponItemsTest, AWeaponIconPointsAtItsOwnRowOfTheSideAtlas)
{
	ASSERT_TRUE(isFound) << "Resources not found from " << previous.string();

	ItemCatalog items = Items();

	for (int index = 0; index < WEAPON_COUNT; index++)
	{
		const ItemDefinition* item = FindWeaponItem(items, WEAPONS[index].id);
		ASSERT_NE(item, nullptr) << WEAPONS[index].id;

		EXPECT_EQ(item->icon.texturePath, "Resources/Textures/weapons_side.png") << WEAPONS[index].id;
		EXPECT_EQ(item->icon.rect.left, 0) << WEAPONS[index].id;
		EXPECT_EQ(item->icon.rect.top, index * SIDE_FRAME_HEIGHT)
			<< "строка атласа разъехалась с перечислением на " << WEAPONS[index].id;
		EXPECT_EQ(item->icon.rect.width, SIDE_FRAME_WIDTH) << WEAPONS[index].id;
		EXPECT_EQ(item->icon.rect.height, SIDE_FRAME_HEIGHT) << WEAPONS[index].id;
	}
}
