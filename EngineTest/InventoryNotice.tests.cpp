#include "pch.h"
#include "EquipExchange.h"
#include "GameSettings.h"
#include "InventoryComponent.h"
#include "InventoryScreen.h"
#include <GameWorld.h>
#include <RenderSystem.h>
#include <TextUtils.h>
#include <UiManager.h>

using RoguelikeGame::EquipRefusal;
using RoguelikeGame::EquipRefuseText;
using RoguelikeGame::InventoryComponent;
using RoguelikeGame::InventoryScreen;
using RoguelikeGame::ItemRefuseReason;
using RoguelikeGame::ItemRefuseText;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::UiManager;

namespace
{
	class InventoryNoticeTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			UiManager::Instance()->Clear();
			XYZEngine::RenderSystem::Instance()->HandleResize(1280, 720);

			GameObject* owner = GameWorld::Instance()->CreateGameObject("Player");
			bag = owner->AddComponent<InventoryComponent>();
			bag->SetCapacity(12);

			screen.Resize({1280.f, 720.f});
			screen.SetInventory(bag);
		}

		void TearDown() override
		{
			UiManager::Instance()->Clear();
			GameWorld::Instance()->Clear();
		}

		InventoryComponent* bag = nullptr;
		InventoryScreen screen;
	};
}

TEST_F(InventoryNoticeTest, TheReasonShowsUpInsideTheBag)
{
	ASSERT_FALSE(screen.GetNotice().IsVisible());

	screen.Open();
	screen.ShowNotice(ItemRefuseText(ItemRefuseReason::NotNeeded));

	EXPECT_TRUE(screen.GetNotice().IsVisible());
	EXPECT_EQ(screen.GetNotice().GetText(), XYZEngine::FromUtf8(RoguelikeGame::ITEM_REFUSED_NOTICE));
}

TEST_F(InventoryNoticeTest, TheReasonFadesByItself)
{
	screen.Open();
	screen.ShowNotice(ItemRefuseText(ItemRefuseReason::NotNeeded));

	for (float spent = 0.f; spent < RoguelikeGame::INVENTORY_NOTICE_TIME + 0.1f; spent += 0.1f)
	{
		screen.Update(0.1f);
	}

	EXPECT_FALSE(screen.GetNotice().IsVisible()) << "надпись осталась висеть навсегда";
}

TEST_F(InventoryNoticeTest, ClosingTheBagTakesTheReasonAway)
{
	screen.Open();
	screen.ShowNotice(ItemRefuseText(ItemRefuseReason::NotNeeded));
	ASSERT_TRUE(screen.GetNotice().IsVisible());

	screen.Close();

	EXPECT_FALSE(screen.GetNotice().IsVisible()) << "прошлый отказ встретит игрока при следующем открытии";
}

TEST_F(InventoryNoticeTest, TheNoticeLineDoesNotCoverTheHint)
{
	screen.Open();
	screen.Update(0.016f);
	screen.ShowNotice(ItemRefuseText(ItemRefuseReason::NotNeeded));

	sf::FloatRect hint = screen.GetHint().GetBounds();
	sf::FloatRect notice = screen.GetNotice().GetBounds();

	EXPECT_FALSE(hint.intersects(notice)) << "сообщение легло поверх подсказки";
	EXPECT_LT(notice.top, hint.top) << "сообщение уехало под подсказку";
}

TEST(EquipRefuseTextTest, EveryRefusalHasItsOwnWords)
{
	const EquipRefusal REFUSALS[] = {EquipRefusal::NotAWeapon, EquipRefusal::UnknownWeapon,
		EquipRefusal::NoSuchSlot, EquipRefusal::WrongKind, EquipRefusal::AlreadyThere, EquipRefusal::NoWayBack};

	for (EquipRefusal refusal : REFUSALS)
	{
		EXPECT_STRNE(EquipRefuseText(refusal), "") << "отказ молчит";
	}

	EXPECT_STRNE(EquipRefuseText(EquipRefusal::WrongKind), EquipRefuseText(EquipRefusal::AlreadyThere));
	EXPECT_STRNE(EquipRefuseText(EquipRefusal::NotAWeapon), EquipRefuseText(EquipRefusal::NoWayBack));
	EXPECT_STREQ(EquipRefuseText(EquipRefusal::None), "");
}
