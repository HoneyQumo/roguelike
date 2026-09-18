#include "pch.h"
#include "GameWorld.h"
#include "InventoryComponent.h"
#include "InventoryScreen.h"
#include "GameSettings.h"
#include "RenderSystem.h"
#include "WeaponCatalog.h"
#include "TextUtils.h"
#include "UiManager.h"
#include "InputSystem.h"

using RoguelikeGame::InventoryComponent;
using RoguelikeGame::InventoryScreen;
using RoguelikeGame::ItemDefinition;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::RenderSystem;
using XYZEngine::UiManager;

namespace
{
	ItemDefinition MakeItem(const std::string& id, const std::string& name, bool stackable = false)
	{
		ItemDefinition item;
		item.id = id;
		item.name = name;
		item.stackable = stackable;
		item.maxStack = stackable ? 5 : 1;
		return item;
	}

	class InventoryScreenTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			UiManager::Instance()->Clear();
			RenderSystem::Instance()->HandleResize(1280, 720);
		}

		void TearDown() override
		{
			UiManager::Instance()->Clear();
			GameWorld::Instance()->Clear();
		}

		InventoryComponent* CreateInventory()
		{
			GameObject* owner = GameWorld::Instance()->CreateGameObject("Player");
			auto inventory = owner->AddComponent<InventoryComponent>();
			inventory->SetCapacity(12);

			return inventory;
		}
	};
}

TEST_F(InventoryScreenTest, ScreenStartsClosedAndHidden)
{
	InventoryScreen screen;

	EXPECT_FALSE(screen.IsOpen());
	EXPECT_FALSE(screen.IsVisible());
}

TEST_F(InventoryScreenTest, OpenShowsScreenAndCloseHidesItAfterAnimation)
{
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.Open();
	EXPECT_TRUE(screen.IsOpen());
	EXPECT_TRUE(screen.IsVisible());

	screen.Close();
	EXPECT_FALSE(screen.IsOpen());

	for (int frame = 0; frame < 30; frame++)
	{
		screen.Update(0.016f);
	}

	EXPECT_FALSE(screen.IsVisible());
}

TEST_F(InventoryScreenTest, SlotsShowItemNameAndCount)
{
	InventoryComponent* inventory = CreateInventory();
	ItemDefinition potion = MakeItem("potion", "Aptechka", true);
	ItemDefinition key = MakeItem("key", "Klyuch");
	inventory->TryAdd(potion, 3);
	inventory->TryAdd(key);

	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);

	EXPECT_EQ(screen.GetSlotName(0).GetText(), sf::String("Aptechka"));
	EXPECT_EQ(screen.GetSlotCount(0).GetText(), sf::String("3"));
	EXPECT_TRUE(screen.GetSlotCount(0).IsVisible());

	EXPECT_EQ(screen.GetSlotName(1).GetText(), sf::String("Klyuch"));
	EXPECT_FALSE(screen.GetSlotCount(1).IsVisible());

	EXPECT_EQ(screen.GetSlotName(2).GetText(), sf::String(""));
}

TEST_F(InventoryScreenTest, SelectedSlotIsHighlighted)
{
	InventoryComponent* inventory = CreateInventory();
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);

	sf::Color unselected = screen.GetSlotPanel(1).GetShape().getFillColor();

	screen.Open();
	sf::Vector2f point = {screen.GetSlotPanel(1).GetBounds().left + 5.f, screen.GetSlotPanel(1).GetBounds().top + 5.f};
	screen.HandlePointer(point, false, true);

	EXPECT_EQ(screen.GetSelectedSlot(), 1);
	EXPECT_NE(screen.GetSlotPanel(1).GetShape().getFillColor(), unselected);
	EXPECT_EQ(inventory->GetSelectedSlot(), 1);
}

TEST_F(InventoryScreenTest, ScreenFollowsInventoryChanges)
{
	InventoryComponent* inventory = CreateInventory();
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);

	ItemDefinition potion = MakeItem("potion", "Aptechka", true);
	inventory->TryAdd(potion, 2);

	EXPECT_EQ(screen.GetSlotName(0).GetText(), sf::String("Aptechka"));
	EXPECT_EQ(screen.GetSlotCount(0).GetText(), sf::String("2"));

	inventory->Remove(0, 2);

	EXPECT_EQ(screen.GetSlotName(0).GetText(), sf::String(""));
	EXPECT_FALSE(screen.GetSlotCount(0).IsVisible());
}

TEST_F(InventoryScreenTest, ClosedScreenIgnoresPointer)
{
	InventoryComponent* inventory = CreateInventory();
	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);

	EXPECT_FALSE(screen.HandlePointer({640.f, 360.f}, false, true));
	EXPECT_EQ(screen.GetSelectedSlot(), 0);
}

namespace
{
	std::string DropPart()
	{
		std::string line = RoguelikeGame::INVENTORY_DROP_HINT;
		std::size_t mark = line.find("%c");

		line.replace(mark, 2, 1, RoguelikeGame::LetterOfKey(
			XYZEngine::InputSystem::Instance()->GetBinding(XYZEngine::InputAction::Drop).key));

		return line;
	}
}

TEST(InventoryHintTest, EmptySlotSaysNothing)
{
	EXPECT_TRUE(RoguelikeGame::InventoryHint(nullptr).empty());
}

// \u0420\u0430\u043d\u044c\u0448\u0435 \u043a\u043b\u044e\u0447\u0443 \u0431\u044b\u043b\u043e \u043d\u0435\u0447\u0435\u0433\u043e \u0441\u043a\u0430\u0437\u0430\u0442\u044c: \u0441 \u043d\u0438\u043c \u043d\u0435\u043b\u044c\u0437\u044f \u0431\u044b\u043b\u043e \u0441\u0434\u0435\u043b\u0430\u0442\u044c \u0440\u043e\u0432\u043d\u043e \u043d\u0438\u0447\u0435\u0433\u043e.
TEST(InventoryHintTest, ItemWithoutAnEffectOffersOnlyToDropIt)
{
	ItemDefinition item = MakeItem("junk", "Hlam");

	EXPECT_EQ(RoguelikeGame::InventoryHint(&item), DropPart());
}

TEST(InventoryHintTest, EveryItemOffersToDropIt)
{
	ItemDefinition potion = MakeItem("potion", "Aptechka");
	potion.effect.kind = RoguelikeGame::ItemEffectKind::Heal;

	ItemDefinition rifle = MakeItem("weapon_ak47", "AK");
	rifle.effect.kind = RoguelikeGame::ItemEffectKind::EquipWeapon;
	rifle.effect.target = "ak47";

	for (const ItemDefinition* item : {&potion, &rifle})
	{
		std::string hint = RoguelikeGame::InventoryHint(item);

		EXPECT_NE(hint.find(DropPart()), std::string::npos) << item->id << ": \u043d\u0435\u0447\u0435\u043c \u0432\u044b\u0431\u0440\u043e\u0441\u0438\u0442\u044c";
	}
}

TEST(InventoryHintTest, ConsumableOffersToUseItAndToHangItOnTheBelt)
{
	ItemDefinition potion = MakeItem("potion", "Aptechka");
	potion.effect.kind = RoguelikeGame::ItemEffectKind::Heal;
	potion.effect.amount = 35.f;

	std::string hint = RoguelikeGame::InventoryHint(&potion);

	EXPECT_EQ(hint.rfind(RoguelikeGame::INVENTORY_USE_HINT, 0), 0u);
	EXPECT_NE(hint, std::string(RoguelikeGame::INVENTORY_USE_HINT)) << "про пояс подсказка молчит";
	EXPECT_NE(hint.find("4"), std::string::npos) << "подсказка не называет клавиши пояса";
}

TEST(InventoryHintTest, WhatIsNotAConsumableIsNotOfferedToTheBelt)
{
	ItemDefinition key = MakeItem("key", "Klyuch");
	key.type = RoguelikeGame::ItemType::Key;
	key.effect.kind = RoguelikeGame::ItemEffectKind::Unlock;

	std::string hint = RoguelikeGame::InventoryHint(&key);

	EXPECT_EQ(hint.rfind(RoguelikeGame::INVENTORY_USE_HINT, 0), 0u);
	EXPECT_EQ(hint.find("4"), std::string::npos) << "\u043f\u043e\u044f\u0441 \u043f\u0440\u0435\u0434\u043b\u043e\u0436\u0435\u043d \u0442\u043e\u043c\u0443, \u0447\u0442\u043e \u043d\u0430 \u043d\u0451\u043c \u043d\u0435 \u0432\u0438\u0441\u0438\u0442";
}

TEST(InventoryHintTest, WeaponNamesTheKeysThatChooseTheSlot)
{
	ItemDefinition rifle = MakeItem("weapon_ak47", "AK");
	rifle.effect.kind = RoguelikeGame::ItemEffectKind::EquipWeapon;
	rifle.effect.target = "ak47";

	ItemDefinition pistol = MakeItem("weapon_deagle", "Deagle");
	pistol.effect.kind = RoguelikeGame::ItemEffectKind::EquipWeapon;
	pistol.effect.target = "deagle";

	std::string hint = RoguelikeGame::InventoryHint(&rifle);

	EXPECT_NE(hint.find(RoguelikeGame::INVENTORY_EQUIP_HINT), std::string::npos);
	EXPECT_NE(hint.find(std::to_string(RoguelikeGame::PLAYER_WEAPON_SLOTS)), std::string::npos)
		<< "подсказка не называет клавиши, которыми выбирают слот";
	EXPECT_EQ(hint, RoguelikeGame::InventoryHint(&pistol)) << "слот выбирает игрок, а не ствол";
}

TEST(InventoryHintTest, UnknownWeaponOffersOnlyToDropIt)
{
	ItemDefinition broken = MakeItem("weapon_ghost", "Prizrak");
	broken.effect.kind = RoguelikeGame::ItemEffectKind::EquipWeapon;
	broken.effect.target = "no_such_gun";

	EXPECT_EQ(RoguelikeGame::InventoryHint(&broken), DropPart());
}

TEST_F(InventoryScreenTest, HintShowsWhatEnterWillDoToTheSelectedSlot)
{
	InventoryComponent* inventory = CreateInventory();

	ItemDefinition potion = MakeItem("potion", "Aptechka");
	potion.effect.kind = RoguelikeGame::ItemEffectKind::Heal;
	potion.effect.amount = 35.f;

	ItemDefinition rifle = MakeItem("weapon_ak47", "AK");
	rifle.effect.kind = RoguelikeGame::ItemEffectKind::EquipWeapon;
	rifle.effect.target = "ak47";

	ASSERT_TRUE(inventory->TryAdd(rifle));
	ASSERT_TRUE(inventory->TryAdd(potion));

	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);
	screen.Open();

	ASSERT_EQ(screen.GetSelectedSlot(), 0);
	EXPECT_EQ(screen.GetHint().GetText(), XYZEngine::FromUtf8(RoguelikeGame::InventoryHint(&rifle).c_str()));

	ASSERT_TRUE(inventory->Remove(0));

	EXPECT_TRUE(screen.GetHint().GetText().isEmpty());
}

TEST_F(InventoryScreenTest, HintPicksUpTheSlotTheInventoryHasSelected)
{
	InventoryComponent* inventory = CreateInventory();

	ItemDefinition potion = MakeItem("potion", "Aptechka");
	potion.effect.kind = RoguelikeGame::ItemEffectKind::Heal;
	potion.effect.amount = 35.f;

	ASSERT_TRUE(inventory->TryAdd(MakeItem("junk", "Hlam")));
	ASSERT_TRUE(inventory->TryAdd(potion));
	inventory->SelectSlot(1);

	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);
	screen.Open();

	ASSERT_EQ(screen.GetSelectedSlot(), 1);
	EXPECT_EQ(screen.GetHint().GetText(), XYZEngine::FromUtf8(RoguelikeGame::InventoryHint(&potion).c_str()));
}


// У ключа есть эффект, но нет обработчика: подсказка звала на мёртвую кнопку.
TEST(InventoryHintTest, WhatHasNoHandlerIsNotOfferedToUse)
{
	ItemDefinition key = MakeItem("key", "Klyuch");
	key.type = RoguelikeGame::ItemType::Key;
	key.effect.kind = RoguelikeGame::ItemEffectKind::Unlock;

	EXPECT_EQ(RoguelikeGame::InventoryHint(&key, false), DropPart()) << "ключ всё ещё предлагают использовать";
}

TEST(InventoryHintTest, WhatHasAHandlerIsStillOfferedToUse)
{
	ItemDefinition potion = MakeItem("potion", "Aptechka");
	potion.effect.kind = RoguelikeGame::ItemEffectKind::Heal;

	std::string hint = RoguelikeGame::InventoryHint(&potion, true);

	EXPECT_EQ(hint.rfind(RoguelikeGame::INVENTORY_USE_HINT, 0), 0u);
}

TEST(InventoryHintTest, WithoutARuleTheHintBehavesAsBefore)
{
	ItemDefinition key = MakeItem("key", "Klyuch");
	key.type = RoguelikeGame::ItemType::Key;
	key.effect.kind = RoguelikeGame::ItemEffectKind::Unlock;

	EXPECT_NE(RoguelikeGame::InventoryHint(&key), DropPart()) << "без правила поведение изменилось";
}

// Экипировка идёт не через обработчик эффекта, поэтому её правило не касается.
TEST(InventoryHintTest, AWeaponIsStillOfferedToEquipWithoutAHandler)
{
	ItemDefinition rifle = MakeItem("weapon_ak47", "AK");
	rifle.effect.kind = RoguelikeGame::ItemEffectKind::EquipWeapon;
	rifle.effect.target = "ak47";

	std::string hint = RoguelikeGame::InventoryHint(&rifle, false);

	EXPECT_NE(hint.find(RoguelikeGame::INVENTORY_EQUIP_HINT), std::string::npos);
}

TEST_F(InventoryScreenTest, TheScreenAsksTheRuleBeforeOfferingToUse)
{
	InventoryComponent* inventory = CreateInventory();

	ItemDefinition key = MakeItem("key", "Klyuch");
	key.type = RoguelikeGame::ItemType::Key;
	key.effect.kind = RoguelikeGame::ItemEffectKind::Unlock;
	inventory->TryAdd(key);

	InventoryScreen screen;
	screen.Resize({1280.f, 720.f});
	screen.SetInventory(inventory);
	screen.Open();

	ASSERT_EQ(screen.GetHint().GetText(), XYZEngine::FromUtf8(RoguelikeGame::InventoryHint(&key).c_str()))
		<< "без правила подсказка должна быть прежней";

	screen.SetUsableRule([](const ItemDefinition&) { return false; });

	// Сравнивать с той же функцией нельзя: обе стороны сломаются разом и тест промолчит.
	sf::String shown = screen.GetHint().GetText();

	EXPECT_EQ(shown.find(XYZEngine::FromUtf8(RoguelikeGame::INVENTORY_USE_HINT)), sf::String::InvalidPos)
		<< "экран не спросил правило";
	EXPECT_FALSE(shown.isEmpty()) << "подсказка пропала совсем, а сброс остаётся";
}
