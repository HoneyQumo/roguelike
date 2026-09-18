#include "pch.h"
#include "GameSettings.h"
#include "QuickBeltComponent.h"
#include <GameWorld.h>
#include <InputSystem.h>

using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemEffect;
using RoguelikeGame::ItemEffectComponent;
using RoguelikeGame::ItemEffectKind;
using RoguelikeGame::ItemType;
using RoguelikeGame::ItemUseResult;
using RoguelikeGame::QuickBeltComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::InputAction;

namespace
{
	ItemDefinition MakeItem(const std::string& id, ItemType type, ItemEffectKind kind)
	{
		ItemDefinition item;
		item.id = id;
		item.name = id;
		item.type = type;
		item.stackable = true;
		item.maxStack = 5;
		item.effect.kind = kind;
		item.effect.amount = 35.f;

		return item;
	}

	ItemDefinition Potion() { return MakeItem("potion", ItemType::Consumable, ItemEffectKind::Heal); }
	ItemDefinition Plate() { return MakeItem("plate", ItemType::Consumable, ItemEffectKind::AddArmor); }
	ItemDefinition Key() { return MakeItem("key", ItemType::Key, ItemEffectKind::Unlock); }

	class QuickBeltTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			owner = GameWorld::Instance()->CreateGameObject("Player");
			bag = owner->AddComponent<InventoryComponent>();
			bag->SetCapacity(12);

			effects = owner->AddComponent<ItemEffectComponent>();
			effects->SetHandler(ItemEffectKind::Heal, [this](const ItemEffect&) -> ItemUseResult { healed++; return true; });
			effects->SetHandler(ItemEffectKind::AddArmor, [](const ItemEffect&) -> ItemUseResult { return true; });

			belt = owner->AddComponent<QuickBeltComponent>();
			belt->SetInventory(bag);
			belt->SetEffects(effects);

			GameWorld::Instance()->Update(0.016f);
			Wait();
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		// Пояс отдыхает между применениями: даём ему остыть.
		void Wait() { belt->Update(RoguelikeGame::QUICK_BELT_COOLDOWN + 0.01f); }

		GameObject* owner = nullptr;
		InventoryComponent* bag = nullptr;
		ItemEffectComponent* effects = nullptr;
		QuickBeltComponent* belt = nullptr;
		int healed = 0;
	};
}

TEST_F(QuickBeltTest, TheFirstMedkitHangsItselfOnAFreeHook)
{
	ASSERT_TRUE(belt->GetBinding(0).empty());

	ASSERT_TRUE(bag->TryAdd(Potion()));

	EXPECT_EQ(belt->GetBinding(0), "potion");
	EXPECT_TRUE(belt->GetBinding(1).empty()) << "одна аптечка заняла два крючка";
}

TEST_F(QuickBeltTest, TheSameItemDoesNotTakeASecondHook)
{
	ASSERT_TRUE(bag->TryAdd(Potion()));
	ASSERT_TRUE(bag->TryAdd(Potion()));

	EXPECT_EQ(belt->GetBinding(0), "potion");
	EXPECT_TRUE(belt->GetBinding(1).empty());
}

TEST_F(QuickBeltTest, DifferentConsumablesTakeDifferentHooks)
{
	ASSERT_TRUE(bag->TryAdd(Potion()));
	ASSERT_TRUE(bag->TryAdd(Plate()));

	EXPECT_EQ(belt->GetBinding(0), "potion");
	EXPECT_EQ(belt->GetBinding(1), "plate");
}

TEST_F(QuickBeltTest, AKeyIsNotWorthAKey)
{
	ASSERT_TRUE(bag->TryAdd(Key()));

	EXPECT_TRUE(belt->GetBinding(0).empty()) << "ключ повесили на крючок мёртвой кнопкой";
}

TEST_F(QuickBeltTest, TheCountOnAHookIsTheOneInTheBag)
{
	ASSERT_TRUE(bag->TryAdd(Potion(), 3));

	EXPECT_EQ(belt->GetCountOn(0), 3);

	ASSERT_TRUE(belt->UseHook(0));

	EXPECT_EQ(belt->GetCountOn(0), 2) << "пояс держит своё число вместо сумкиного";
	EXPECT_EQ(healed, 1);
}

TEST_F(QuickBeltTest, AHookSurvivesRunningOutAndLightsUpAgain)
{
	ASSERT_TRUE(bag->TryAdd(Potion()));
	ASSERT_TRUE(belt->UseHook(0));

	EXPECT_EQ(belt->GetBinding(0), "potion") << "крючок отвязался, когда аптечка кончилась";
	EXPECT_EQ(belt->GetCountOn(0), 0);
	Wait();
	EXPECT_FALSE(belt->UseHook(0));

	ASSERT_TRUE(bag->TryAdd(Potion()));
	Wait();

	EXPECT_EQ(belt->GetCountOn(0), 1);
	EXPECT_TRUE(belt->UseHook(0));
}

TEST_F(QuickBeltTest, AnEmptyHookIsSilent)
{
	EXPECT_FALSE(belt->UseHook(0));
	EXPECT_FALSE(belt->UseHook(-1));
	EXPECT_FALSE(belt->UseHook(RoguelikeGame::QUICK_BELT_HOOKS));
	EXPECT_EQ(healed, 0);
}

TEST_F(QuickBeltTest, OnePressDoesNotDrinkTwo)
{
	ASSERT_TRUE(bag->TryAdd(Potion(), 3));

	ASSERT_TRUE(belt->UseHook(0));
	EXPECT_FALSE(belt->UseHook(0)) << "второй кадр выпил ещё одну";
	EXPECT_EQ(healed, 1);

	Wait();

	EXPECT_TRUE(belt->UseHook(0));
	EXPECT_EQ(healed, 2);
}

TEST_F(QuickBeltTest, AFrozenPlayerDrinksNothing)
{
	ASSERT_TRUE(bag->TryAdd(Potion()));

	bag->SetEnabled(false);

	EXPECT_FALSE(belt->UseHook(0)) << "лечение проходит на замороженном игроке";
	EXPECT_EQ(healed, 0);

	bag->SetEnabled(true);
	Wait();

	EXPECT_TRUE(belt->UseHook(0)) << "пояс не ожил после разморозки";
}

TEST(QuickBeltInputTest, TheNewActionsSitAtTheTailAndKeepTheOldArithmetic)
{
	EXPECT_EQ(static_cast<int>(InputAction::WeaponSlot1) + 1, static_cast<int>(InputAction::WeaponSlot2));
	EXPECT_EQ(static_cast<int>(InputAction::WeaponSlot1) + 2, static_cast<int>(InputAction::WeaponSlot3));

	EXPECT_EQ(static_cast<int>(InputAction::QuickSlot1) + 1, static_cast<int>(InputAction::QuickSlot2));
	EXPECT_EQ(static_cast<int>(InputAction::QuickSlot1) + 2, static_cast<int>(InputAction::QuickSlot3));

	EXPECT_GT(static_cast<int>(InputAction::QuickSlot1), static_cast<int>(InputAction::Confirm))
		<< "новые действия вставлены в середину - привязки разъедутся молча";
	EXPECT_EQ(static_cast<int>(InputAction::QuickSlot3) + 1, static_cast<int>(InputAction::Count));
}

TEST(QuickBeltInputTest, EveryHookHasAKeyOfItsOwn)
{
	auto input = XYZEngine::InputSystem::Instance();

	for (int hook = 0; hook < RoguelikeGame::QUICK_BELT_HOOKS; hook++)
	{
		auto action = static_cast<InputAction>(static_cast<int>(InputAction::QuickSlot1) + hook);

		EXPECT_NE(input->GetBinding(action).key, sf::Keyboard::Unknown) << "крючок " << hook << " без клавиши";
	}
}
