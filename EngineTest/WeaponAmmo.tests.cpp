#include "pch.h"
#include "ItemCatalogLoader.h"
#include "ProjectFiles.h"
#include "WeaponCatalog.h"
#include "WeaponComponent.h"
#include <GameWorld.h>

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

namespace
{
	int LowMarkFor(int magazineSize)
	{
		XYZEngine::GameObject* owner = XYZEngine::GameWorld::Instance()->CreateGameObject("Gun");
		auto weapon = owner->AddComponent<RoguelikeGame::WeaponComponent>();
		weapon->SetMagazine(magazineSize, 0);

		return weapon->GetLowMagazineMark();
	}
}

TEST(WeaponLowMagazineTest, ARifleWarnsWithAlmostTenRoundsLeft)
{
	XYZEngine::GameWorld::Instance()->Clear();

	EXPECT_EQ(LowMarkFor(30), 9);
	EXPECT_EQ(LowMarkFor(17), 5);

	XYZEngine::GameWorld::Instance()->Clear();
}

TEST(WeaponLowMagazineTest, ASmallMagazineStillWarns)
{
	XYZEngine::GameWorld::Instance()->Clear();

	// Треть от шести - два, а от двух уже ноль: нижняя граница держит смысл.
	EXPECT_EQ(LowMarkFor(6), 2);
	EXPECT_EQ(LowMarkFor(2), 1);

	XYZEngine::GameWorld::Instance()->Clear();
}

TEST(WeaponLowMagazineTest, ASingleShotWeaponIsLowOnlyWhenEmpty)
{
	XYZEngine::GameWorld::Instance()->Clear();

	XYZEngine::GameObject* owner = XYZEngine::GameWorld::Instance()->CreateGameObject("Rpg");
	auto weapon = owner->AddComponent<RoguelikeGame::WeaponComponent>();
	weapon->SetMagazine(1, 0);
	weapon->SetAmmoInMagazine(1);

	EXPECT_FALSE(weapon->IsMagazineLow()) << "a loaded rocket counts as running out";

	weapon->SetAmmoInMagazine(0);
	EXPECT_TRUE(weapon->IsMagazineLow());

	XYZEngine::GameWorld::Instance()->Clear();
}

TEST(WeaponLowMagazineTest, AMeleeWeaponIsNeverLow)
{
	XYZEngine::GameWorld::Instance()->Clear();

	XYZEngine::GameObject* owner = XYZEngine::GameWorld::Instance()->CreateGameObject("Knife");
	auto weapon = owner->AddComponent<RoguelikeGame::WeaponComponent>();

	EXPECT_FALSE(weapon->IsMagazineLow());
	EXPECT_EQ(weapon->GetLowMagazineMark(), 0);

	XYZEngine::GameWorld::Instance()->Clear();
}
