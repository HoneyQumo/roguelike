#include "pch.h"
#include "GameWorld.h"
#include "HealthComponent.h"
#include "InventoryComponent.h"
#include "ItemEffectComponent.h"
#include "GameSettings.h"

using RoguelikeGame::HealthComponent;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemEffect;
using RoguelikeGame::ItemEffectComponent;
using RoguelikeGame::ItemEffectKind;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	ItemDefinition MakeItem(const std::string& id, ItemEffectKind kind, float amount = 0.f, const std::string& target = "")
	{
		ItemDefinition item;
		item.id = id;
		item.name = id;
		item.effect.kind = kind;
		item.effect.amount = amount;
		item.effect.target = target;

		return item;
	}

	class ItemEffectTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			player = GameWorld::Instance()->CreateGameObject("Player");
			health = player->AddComponent<HealthComponent>();
			health->SetMaxHealth(100.f);

			inventory = player->AddComponent<InventoryComponent>();
			inventory->SetCapacity(4);

			effects = player->AddComponent<ItemEffectComponent>();

			GameWorld::Instance()->Update(0.016f);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* player = nullptr;
		HealthComponent* health = nullptr;
		InventoryComponent* inventory = nullptr;
		ItemEffectComponent* effects = nullptr;
	};
}

TEST_F(ItemEffectTest, UnknownEffectIsRefused)
{
	ItemDefinition potion = MakeItem("potion", ItemEffectKind::Heal, 30.f);

	int refusals = 0;
	effects->SubscribeRefused([&refusals](const ItemDefinition&, RoguelikeGame::ItemRefuseReason) { refusals++; });

	EXPECT_FALSE(effects->Apply(potion).isApplied);
	EXPECT_EQ(refusals, 1);
}

TEST_F(ItemEffectTest, RegisteredHandlerTakesTheEffect)
{
	float healed = 0.f;
	effects->SetHandler(ItemEffectKind::Heal, [&healed](const ItemEffect& effect)
	{
		healed = effect.amount;
		return true;
	});

	ItemDefinition potion = MakeItem("potion", ItemEffectKind::Heal, 30.f);

	EXPECT_TRUE(effects->Apply(potion).isApplied);
	EXPECT_FLOAT_EQ(healed, 30.f);
	EXPECT_TRUE(effects->HasHandler(ItemEffectKind::Heal));
}

TEST_F(ItemEffectTest, HandlerCanRefuseTheEffect)
{
	effects->SetHandler(ItemEffectKind::Unlock, [](const ItemEffect&) { return false; });

	ItemDefinition key = MakeItem("key", ItemEffectKind::Unlock, 0.f, "door");

	int refusals = 0;
	effects->SubscribeRefused([&refusals](const ItemDefinition&, RoguelikeGame::ItemRefuseReason) { refusals++; });

	EXPECT_FALSE(effects->Apply(key).isApplied);
	EXPECT_EQ(refusals, 1);
}

TEST_F(ItemEffectTest, NewHandlerReplacesTheOldOne)
{
	int first = 0;
	int second = 0;
	effects->SetHandler(ItemEffectKind::Heal, [&first](const ItemEffect&) { first++; return true; });
	effects->SetHandler(ItemEffectKind::Heal, [&second](const ItemEffect&) { second++; return true; });

	effects->Apply(MakeItem("potion", ItemEffectKind::Heal, 10.f));

	EXPECT_EQ(first, 0);
	EXPECT_EQ(second, 1);
}

TEST_F(ItemEffectTest, UsedItemIsSpentOnlyWhenTheEffectWorked)
{
	static const ItemDefinition potion = MakeItem("potion", ItemEffectKind::Heal, 40.f);
	effects->SetHandler(ItemEffectKind::Heal, [this](const ItemEffect& effect)
	{
		return health->Heal(effect.amount) > 0.f;
	});

	ASSERT_TRUE(inventory->TryAdd(potion, 2));

	health->TakeDamage(50.f);
	EXPECT_TRUE(inventory->Use(0));
	EXPECT_EQ(inventory->CountOf("potion"), 1);
	EXPECT_FLOAT_EQ(health->GetHealth(), 90.f);

	EXPECT_FALSE(inventory->Use(0));
	EXPECT_EQ(inventory->CountOf("potion"), 1);
}

TEST_F(ItemEffectTest, ItemWithoutAnyHandlerStaysInTheBag)
{
	static const ItemDefinition key = MakeItem("key", ItemEffectKind::Unlock, 0.f, "door");
	ASSERT_TRUE(inventory->TryAdd(key, 1));

	EXPECT_FALSE(inventory->Use(0));
	EXPECT_EQ(inventory->CountOf("key"), 1);
}

TEST_F(ItemEffectTest, AppliedItemIsReported)
{
	static const ItemDefinition potion = MakeItem("potion", ItemEffectKind::Heal, 25.f);
	effects->SetHandler(ItemEffectKind::Heal, [](const ItemEffect&) { return true; });

	std::string applied;
	effects->SubscribeApplied([&applied](const ItemDefinition& item) { applied = item.id; });

	effects->Apply(potion);

	EXPECT_EQ(applied, "potion");
}

TEST_F(ItemEffectTest, InventoryWithoutHandlerKeepsWorkingAsBefore)
{
	static const ItemDefinition potion = MakeItem("potion", ItemEffectKind::Heal, 25.f);

	GameObject* other = GameWorld::Instance()->CreateGameObject("Chest");
	auto bag = other->AddComponent<InventoryComponent>();
	bag->SetCapacity(2);
	ASSERT_TRUE(bag->TryAdd(potion, 1));

	EXPECT_TRUE(bag->Use(0));
	EXPECT_EQ(bag->CountOf("potion"), 0);
}

TEST(ArmorRulesTest, PlateRaisesArmorUpToTheCap)
{
	EXPECT_FLOAT_EQ(RoguelikeGame::ArmorAfterPlate(5.f, 5.f, 20.f), 10.f);
	EXPECT_FLOAT_EQ(RoguelikeGame::ArmorAfterPlate(18.f, 5.f, 20.f), 20.f);
}

TEST(ArmorRulesTest, AtTheCapNothingChanges)
{
	EXPECT_FLOAT_EQ(RoguelikeGame::ArmorAfterPlate(20.f, 5.f, 20.f), 20.f);
	EXPECT_FLOAT_EQ(RoguelikeGame::ArmorAfterPlate(25.f, 5.f, 20.f), 25.f);
	EXPECT_FALSE(RoguelikeGame::CanTakeArmor(20.f, 20.f));
	EXPECT_TRUE(RoguelikeGame::CanTakeArmor(19.f, 20.f));
}

TEST(ArmorRulesTest, EmptyPlateChangesNothing)
{
	EXPECT_FLOAT_EQ(RoguelikeGame::ArmorAfterPlate(5.f, 0.f, 20.f), 5.f);
	EXPECT_FLOAT_EQ(RoguelikeGame::ArmorAfterPlate(5.f, -3.f, 20.f), 5.f);
}

TEST_F(ItemEffectTest, ArmorPlateIsSpentOnlyWhileItHelps)
{
	static const ItemDefinition plate = MakeItem("armor_plate", ItemEffectKind::AddArmor, 5.f);

	float armor = 5.f;
	effects->SetHandler(ItemEffectKind::AddArmor, [&armor](const RoguelikeGame::ItemEffect& effect)
	{
		float taken = RoguelikeGame::ArmorAfterPlate(armor, effect.amount, RoguelikeGame::PLAYER_ARMOR_CAP);
		if (taken <= armor)
		{
			return false;
		}

		armor = taken;
		return true;
	});

	ASSERT_TRUE(inventory->TryAdd(plate, 1));
	EXPECT_TRUE(inventory->Use(0));
	EXPECT_FLOAT_EQ(armor, 10.f);
	EXPECT_EQ(inventory->CountOf("armor_plate"), 0);

	armor = RoguelikeGame::PLAYER_ARMOR_CAP;
	ASSERT_TRUE(inventory->TryAdd(plate, 1));
	EXPECT_FALSE(inventory->Use(0));
	EXPECT_EQ(inventory->CountOf("armor_plate"), 1);
}
