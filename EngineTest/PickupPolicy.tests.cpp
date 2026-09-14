#include "pch.h"
#include "GameWorld.h"
#include "BoxColliderComponent.h"
#include "FactionComponent.h"
#include "InventoryComponent.h"
#include "ItemEffectComponent.h"
#include "ItemPickupComponent.h"

using RoguelikeGame::Faction;
using RoguelikeGame::FactionComponent;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemEffect;
using RoguelikeGame::ItemEffectComponent;
using RoguelikeGame::ItemEffectKind;
using RoguelikeGame::ItemPickupComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	ItemDefinition MakeItem(const std::string& id, ItemEffectKind kind, const std::string& target = "")
	{
		ItemDefinition item;
		item.id = id;
		item.name = id;
		item.stackable = false;
		item.maxStack = 1;
		item.effect.kind = kind;
		item.effect.target = target;

		return item;
	}

	class PickupPolicyTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			player = GameWorld::Instance()->CreateGameObject("Player");
			player->AddComponent<FactionComponent>()->SetFaction(Faction::Player);
			inventory = player->AddComponent<InventoryComponent>();
			inventory->SetCapacity(4);
			effects = player->AddComponent<ItemEffectComponent>();

			GameWorld::Instance()->Update(0.016f);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		ItemPickupComponent* DropItem(const ItemDefinition& definition)
		{
			GameObject* item = GameWorld::Instance()->CreateGameObject("Item_" + definition.id);
			auto collider = item->AddComponent<XYZEngine::BoxColliderComponent>();
			collider->SetSize(40.f, 40.f);
			collider->SetTrigger(true);

			auto pickup = item->AddComponent<ItemPickupComponent>();
			pickup->SetDefinition(&definition);

			GameWorld::Instance()->Update(0.016f);

			return pickup;
		}

		GameObject* player = nullptr;
		InventoryComponent* inventory = nullptr;
		ItemEffectComponent* effects = nullptr;
	};
}

TEST_F(PickupPolicyTest, WithoutARuleTheItemGoesToTheBag)
{
	static const ItemDefinition ammo = MakeItem("ammo", ItemEffectKind::AddAmmo, "pistol");

	int applied = 0;
	effects->SetHandler(ItemEffectKind::AddAmmo, [&](const ItemEffect&) { applied++; return true; });

	ItemPickupComponent* pickup = DropItem(ammo);

	EXPECT_TRUE(pickup->TryPickUp(player));
	EXPECT_EQ(applied, 0);
	EXPECT_EQ(inventory->CountOf("ammo"), 1);
}

TEST_F(PickupPolicyTest, RuleSendsTheItemStraightToTheEffect)
{
	static const ItemDefinition ammo = MakeItem("ammo", ItemEffectKind::AddAmmo, "pistol");

	int applied = 0;
	effects->SetHandler(ItemEffectKind::AddAmmo, [&](const ItemEffect&) { applied++; return true; });
	effects->SetPickupRule(ItemEffectKind::AddAmmo, [](const ItemEffect&) { return true; });

	ItemPickupComponent* pickup = DropItem(ammo);

	EXPECT_TRUE(pickup->TryPickUp(player));
	EXPECT_EQ(applied, 1);
	EXPECT_EQ(inventory->CountOf("ammo"), 0);
	EXPECT_EQ(inventory->GetUsedSlots(), 0);
}

TEST_F(PickupPolicyTest, RuleThatSaysNoLeavesTheItemToTheBag)
{
	static const ItemDefinition gun = MakeItem("gun", ItemEffectKind::EquipWeapon, "deagle");

	int applied = 0;
	effects->SetHandler(ItemEffectKind::EquipWeapon, [&](const ItemEffect&) { applied++; return true; });
	effects->SetPickupRule(ItemEffectKind::EquipWeapon, [](const ItemEffect&) { return false; });

	ItemPickupComponent* pickup = DropItem(gun);

	EXPECT_TRUE(pickup->TryPickUp(player));
	EXPECT_EQ(applied, 0);
	EXPECT_EQ(inventory->CountOf("gun"), 1);
}

TEST_F(PickupPolicyTest, RuleLooksAtTheEffectItself)
{
	static const ItemDefinition known = MakeItem("known", ItemEffectKind::EquipWeapon, "deagle");
	static const ItemDefinition ghost = MakeItem("ghost", ItemEffectKind::EquipWeapon, "no_such_gun");

	effects->SetHandler(ItemEffectKind::EquipWeapon, [](const ItemEffect&) { return true; });
	effects->SetPickupRule(ItemEffectKind::EquipWeapon, [](const ItemEffect& effect) { return effect.target == "deagle"; });

	ASSERT_TRUE(DropItem(known)->TryPickUp(player));
	ASSERT_TRUE(DropItem(ghost)->TryPickUp(player));

	EXPECT_EQ(inventory->CountOf("known"), 0);
	EXPECT_EQ(inventory->CountOf("ghost"), 1);
}

TEST_F(PickupPolicyTest, FailedEffectLeavesTheItemOnTheFloor)
{
	static const ItemDefinition ammo = MakeItem("ammo", ItemEffectKind::AddAmmo, "pistol");

	effects->SetHandler(ItemEffectKind::AddAmmo, [](const ItemEffect&) { return false; });
	effects->SetPickupRule(ItemEffectKind::AddAmmo, [](const ItemEffect&) { return true; });

	ItemPickupComponent* pickup = DropItem(ammo);

	EXPECT_FALSE(pickup->TryPickUp(player));
	EXPECT_FALSE(pickup->IsPickedUp());
	EXPECT_TRUE(pickup->IsAvailable());
	EXPECT_EQ(inventory->GetUsedSlots(), 0);
}

TEST_F(PickupPolicyTest, ItemWithARuleButNoHandlerGoesToTheBag)
{
	static const ItemDefinition ammo = MakeItem("ammo", ItemEffectKind::AddAmmo, "pistol");

	effects->SetPickupRule(ItemEffectKind::AddAmmo, [](const ItemEffect&) { return true; });

	ItemPickupComponent* pickup = DropItem(ammo);

	EXPECT_TRUE(pickup->TryPickUp(player));
	EXPECT_EQ(inventory->CountOf("ammo"), 1);
}

TEST_F(PickupPolicyTest, FullBagStillRefusesAnItemWithoutARule)
{
	static const ItemDefinition junk = MakeItem("junk", ItemEffectKind::None);

	inventory->SetCapacity(1);
	ASSERT_TRUE(inventory->TryAdd(MakeItem("stone", ItemEffectKind::None)));

	ItemPickupComponent* pickup = DropItem(junk);

	EXPECT_FALSE(pickup->TryPickUp(player));
	EXPECT_FALSE(pickup->IsPickedUp());
}
