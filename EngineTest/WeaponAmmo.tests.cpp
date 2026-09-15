#include "pch.h"
#include "ItemCatalogLoader.h"
#include "ProjectFiles.h"
#include "WeaponCatalog.h"

using RoguelikeGame::AmmoKind;
using RoguelikeGame::ItemCatalog;
using RoguelikeGame::ItemCatalogLoader;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemEffectKind;
using RoguelikeGame::WeaponDefinition;
using RoguelikeGame::WEAPON_COUNT;
using RoguelikeGame::WEAPONS;

namespace
{
	const ItemDefinition* FindAmmoSource(const ItemCatalog& items, AmmoKind kind)
	{
		for (const ItemDefinition& item : items)
		{
			AmmoKind carried = AmmoKind::None;
			if (item.effect.kind == ItemEffectKind::AddAmmo
				&& RoguelikeGame::TryGetAmmoKind(item.effect.target, carried) && carried == kind)
			{
				return &item;
			}
		}

		return nullptr;
	}

	class ShippedAmmoTest : public ProjectFiles::Test
	{
	};
}

TEST_F(ShippedAmmoTest, EveryAmmoKindAWeaponEatsCanBeFoundInTheWorld)
{
	ASSERT_TRUE(isFound) << "Resources not found from " << previous.string();

	ItemCatalog items = ItemCatalogLoader::Load("Resources/Items/items.config");
	ASSERT_GT(items.Size(), 0u);

	for (int index = 0; index < WEAPON_COUNT; index++)
	{
		const WeaponDefinition& weapon = WEAPONS[index];
		if (weapon.ammo == AmmoKind::None)
		{
			continue;
		}

		EXPECT_NE(FindAmmoSource(items, weapon.ammo), nullptr) << weapon.id << " eats ammo nothing in the world carries";
	}
}

TEST_F(ShippedAmmoTest, APackRefillsAtLeastOneMagazine)
{
	ASSERT_TRUE(isFound) << "Resources not found from " << previous.string();

	ItemCatalog items = ItemCatalogLoader::Load("Resources/Items/items.config");
	ASSERT_GT(items.Size(), 0u);

	for (int index = 0; index < WEAPON_COUNT; index++)
	{
		const WeaponDefinition& weapon = WEAPONS[index];
		if (weapon.ammo == AmmoKind::None || weapon.magazineSize <= 0)
		{
			continue;
		}

		const ItemDefinition* pack = FindAmmoSource(items, weapon.ammo);
		ASSERT_NE(pack, nullptr) << weapon.id;

		EXPECT_GE(pack->effect.amount, static_cast<float>(weapon.magazineSize))
			<< "a pack of " << pack->id << " does not fill the magazine of " << weapon.id;
	}
}
