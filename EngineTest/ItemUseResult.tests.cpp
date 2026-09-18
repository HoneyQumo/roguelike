#include "pch.h"
#include "GameSettings.h"
#include "GameWorld.h"
#include "InventoryComponent.h"
#include "ItemEffectComponent.h"

using RoguelikeGame::InventoryComponent;
using RoguelikeGame::ItemDefinition;
using RoguelikeGame::ItemEffect;
using RoguelikeGame::ItemEffectComponent;
using RoguelikeGame::ItemEffectKind;
using RoguelikeGame::ItemRefuseReason;
using RoguelikeGame::ItemRefuseText;
using RoguelikeGame::ItemUseResult;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	ItemDefinition Potion()
	{
		ItemDefinition item;
		item.id = "potion";
		item.name = "Potion";
		item.stackable = true;
		item.maxStack = 5;
		item.effect.kind = ItemEffectKind::Heal;
		item.effect.amount = 35.f;

		return item;
	}

	class ItemUseResultTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			GameObject* player = GameWorld::Instance()->CreateGameObject("Player");
			bag = player->AddComponent<InventoryComponent>();
			bag->SetCapacity(4);
			effects = player->AddComponent<ItemEffectComponent>();

			GameWorld::Instance()->Update(0.016f);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		InventoryComponent* bag = nullptr;
		ItemEffectComponent* effects = nullptr;
	};
}

TEST_F(ItemUseResultTest, ARefusalCarriesItsReason)
{
	effects->SetHandler(ItemEffectKind::Heal, [](const ItemEffect&) -> ItemUseResult
	{
		return {false, 0, ItemRefuseReason::NotNeeded};
	});

	ItemRefuseReason heard = ItemRefuseReason::None;
	effects->SubscribeRefused([&heard](const ItemDefinition&, ItemRefuseReason reason) { heard = reason; });

	ItemUseResult result = effects->Apply(Potion());

	EXPECT_FALSE(result.isApplied);
	EXPECT_EQ(result.reason, ItemRefuseReason::NotNeeded);
	EXPECT_EQ(heard, ItemRefuseReason::NotNeeded) << "отказ по-прежнему немой";
}

TEST_F(ItemUseResultTest, AnItemNobodyKnowsHowToApplySaysSo)
{
	ItemRefuseReason heard = ItemRefuseReason::None;
	effects->SubscribeRefused([&heard](const ItemDefinition&, ItemRefuseReason reason) { heard = reason; });

	ItemUseResult result = effects->Apply(Potion());

	EXPECT_EQ(result.reason, ItemRefuseReason::NoHandler);
	EXPECT_EQ(heard, ItemRefuseReason::NoHandler);
}

TEST_F(ItemUseResultTest, ARefusedItemStaysInTheBag)
{
	effects->SetHandler(ItemEffectKind::Heal, [](const ItemEffect&) -> ItemUseResult
	{
		return {false, 0, ItemRefuseReason::NotNeeded};
	});

	ASSERT_TRUE(bag->TryAdd(Potion()));

	EXPECT_FALSE(bag->Use(0));
	EXPECT_EQ(bag->CountOf("potion"), 1) << "отказ съел предмет";
}

TEST_F(ItemUseResultTest, AnEffectSpendsExactlyWhatItAsked)
{
	effects->SetHandler(ItemEffectKind::Heal, [](const ItemEffect&) -> ItemUseResult
	{
		return {true, 2, ItemRefuseReason::None};
	});

	ASSERT_TRUE(bag->TryAdd(Potion(), 3));
	ASSERT_TRUE(bag->Use(0));

	EXPECT_EQ(bag->CountOf("potion"), 1) << "сумка списала своё число, а не то, что попросил эффект";
}

TEST_F(ItemUseResultTest, AnEffectThatSpendsNothingLeavesTheCellAlone)
{
	effects->SetHandler(ItemEffectKind::Heal, [](const ItemEffect&) -> ItemUseResult
	{
		return {true, 0, ItemRefuseReason::None};
	});

	int used = 0;
	bag->SubscribeUsed([&used](const ItemDefinition&) { used++; });

	ASSERT_TRUE(bag->TryAdd(Potion()));

	EXPECT_FALSE(bag->Use(0));
	EXPECT_EQ(bag->CountOf("potion"), 1);
	EXPECT_EQ(used, 0) << "о трате сообщили, хотя ничего не потратили";
}

TEST_F(ItemUseResultTest, AHandlerThatAnswersYesOrNoKeepsWorkingAsBefore)
{
	effects->SetHandler(ItemEffectKind::Heal, [](const ItemEffect&) { return true; });

	ASSERT_TRUE(bag->TryAdd(Potion(), 2));
	ASSERT_TRUE(bag->Use(0));
	EXPECT_EQ(bag->CountOf("potion"), 1) << "старое да перестало списывать одну штуку";

	effects->SetHandler(ItemEffectKind::Heal, [](const ItemEffect&) { return false; });

	EXPECT_FALSE(bag->Use(0));
	EXPECT_EQ(bag->CountOf("potion"), 1) << "старое нет перестало беречь предмет";
}

TEST(ItemRefuseTextTest, EveryReasonHasItsOwnWords)
{
	EXPECT_STRNE(ItemRefuseText(ItemRefuseReason::NoHandler), ItemRefuseText(ItemRefuseReason::NotNeeded));
	EXPECT_STRNE(ItemRefuseText(ItemRefuseReason::Unknown), ItemRefuseText(ItemRefuseReason::NotNeeded));
	EXPECT_STRNE(ItemRefuseText(ItemRefuseReason::NoHandler), ItemRefuseText(ItemRefuseReason::Unknown));

	EXPECT_STRNE(ItemRefuseText(ItemRefuseReason::None), "");
}
