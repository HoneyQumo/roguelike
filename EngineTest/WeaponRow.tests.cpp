#include "pch.h"
#include "GameSettings.h"
#include "HudScreen.h"
#include <RenderSystem.h>
#include <TextUtils.h>

using RoguelikeGame::HudScreen;
using RoguelikeGame::SlotHudState;

namespace
{
	std::vector<SlotHudState> Loadout(int count, int current, bool isArmed = true)
	{
		std::vector<SlotHudState> slots;

		for (int slot = 0; slot < count; slot++)
		{
			SlotHudState shown;
			shown.isFilled = isArmed || slot == current;
			shown.isCurrent = slot == current;
			shown.key = slot + 1;

			slots.push_back(shown);
		}

		return slots;
	}

	class WeaponRowTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			XYZEngine::RenderSystem::Instance()->HandleResize(1280, 720);
			screen.Resize({1280.f, 720.f});
		}

		HudScreen screen;
	};
}

TEST_F(WeaponRowTest, TheRowShowsAsManySlotsAsTheLoadoutHas)
{
	screen.SetWeaponSlots(Loadout(2, 0));

	EXPECT_EQ(screen.GetWeaponSlotsShown(), 2) << "число ячеек взято из экрана, а не из раскладки";

	screen.SetWeaponSlots(Loadout(RoguelikeGame::PLAYER_WEAPON_SLOTS, 0));

	EXPECT_EQ(screen.GetWeaponSlotsShown(), RoguelikeGame::PLAYER_WEAPON_SLOTS);
}

TEST_F(WeaponRowTest, ExactlyOneSlotIsLit)
{
	screen.SetWeaponSlots(Loadout(RoguelikeGame::PLAYER_WEAPON_SLOTS, 1));

	int lit = 0;
	for (int slot = 0; slot < RoguelikeGame::PLAYER_WEAPON_SLOTS; slot++)
	{
		lit += screen.GetWeaponSlotPanel(slot).GetShape().getFillColor() == RoguelikeGame::INVENTORY_SLOT_SELECTED_COLOR ? 1 : 0;
	}

	EXPECT_EQ(lit, 1) << "подсвечен не тот слот, что в руках";
}

TEST_F(WeaponRowTest, AnEmptySlotKeepsItsKeyAndHidesItsIcon)
{
	std::vector<SlotHudState> slots = Loadout(RoguelikeGame::PLAYER_WEAPON_SLOTS, 2, false);
	screen.SetWeaponSlots(slots);

	EXPECT_FALSE(screen.GetWeaponSlotIcon(0).IsVisible()) << "в пустом слоте рисуется оружие";
	EXPECT_EQ(screen.GetWeaponSlotKey(0).GetText(), XYZEngine::FromUtf8("1")) << "пустой слот потерял свою цифру";
	EXPECT_NE(screen.GetWeaponSlotPanel(0).GetShape().getFillColor(), RoguelikeGame::INVENTORY_SLOT_FILLED_COLOR);
}

TEST_F(WeaponRowTest, TheKeyComesFromTheStateAndNotFromTheCellNumber)
{
	std::vector<SlotHudState> slots = Loadout(2, 0);
	slots[1].key = 7;

	screen.SetWeaponSlots(slots);

	EXPECT_EQ(screen.GetWeaponSlotKey(1).GetText(), XYZEngine::FromUtf8("7")) << "подпись написана руками, а не взята из привязки";
}

TEST_F(WeaponRowTest, TheRowKeepsClearOfTheVitalsAndThePrompt)
{
	screen.SetWeaponSlots(Loadout(RoguelikeGame::PLAYER_WEAPON_SLOTS, 0));
	screen.SetVitals({1.f, 1.f, 0.f, false});

	for (int slot = 0; slot < RoguelikeGame::PLAYER_WEAPON_SLOTS; slot++)
	{
		sf::FloatRect cell = screen.GetWeaponSlotPanel(slot).GetBounds();

		EXPECT_FALSE(cell.intersects(screen.GetHealthBar().GetBounds())) << "ряд наехал на полосу жизни, слот " << slot;
		EXPECT_FALSE(cell.intersects(screen.GetPromptLabel().GetBounds())) << "ряд наехал на подсказку, слот " << slot;
	}
}

TEST_F(WeaponRowTest, AWeaponShowsItsMagazineAndReserveInTheCell)
{
	std::vector<SlotHudState> slots = Loadout(2, 0);
	slots[0].hasCount = true;
	slots[0].count = 30;
	slots[0].reserve = 90;

	screen.SetWeaponSlots(slots);

	EXPECT_TRUE(screen.GetWeaponSlotCount(0).IsVisible());
	EXPECT_EQ(screen.GetWeaponSlotCount(0).GetText(), XYZEngine::FromUtf8("30/90"));
	EXPECT_FALSE(screen.GetWeaponSlotCount(1).IsVisible()) << "у ствола без магазина взялся счётчик";
}

TEST_F(WeaponRowTest, AStackShowsOneNumberWithoutAReserve)
{
	std::vector<SlotHudState> slots = Loadout(1, 0);
	slots[0].hasCount = true;
	slots[0].count = 3;
	slots[0].reserve = RoguelikeGame::NO_RESERVE;

	screen.SetWeaponSlots(slots);

	EXPECT_EQ(screen.GetWeaponSlotCount(0).GetText(), XYZEngine::FromUtf8("3")) << "стопке пририсовали запас";
}

TEST_F(WeaponRowTest, TheCountTellsReloadingAndLowApart)
{
	std::vector<SlotHudState> slots = Loadout(1, 0);
	slots[0].hasCount = true;
	slots[0].count = 2;
	slots[0].reserve = 90;

	screen.SetWeaponSlots(slots);
	sf::Color plain = screen.GetWeaponSlotCount(0).GetColor();

	slots[0].isLow = true;
	screen.SetWeaponSlots(slots);
	sf::Color low = screen.GetWeaponSlotCount(0).GetColor();

	slots[0].isReloading = true;
	screen.SetWeaponSlots(slots);
	sf::Color reloading = screen.GetWeaponSlotCount(0).GetColor();

	EXPECT_NE(plain, low) << "малый магазин не отличить";
	EXPECT_NE(low, reloading) << "перезарядку не отличить от малого магазина";
}

TEST_F(WeaponRowTest, TheCountDoesNotCoverTheKey)
{
	std::vector<SlotHudState> slots = Loadout(1, 0);
	slots[0].hasCount = true;
	slots[0].count = 30;
	slots[0].reserve = 90;

	screen.SetWeaponSlots(slots);

	EXPECT_FALSE(screen.GetWeaponSlotKey(0).GetBounds().intersects(screen.GetWeaponSlotCount(0).GetBounds()))
		<< "цифра клавиши и счётчик толкаются";
}

TEST_F(WeaponRowTest, TheBeltIsItsOwnGroupBesideTheWeapons)
{
	screen.SetWeaponSlots(Loadout(RoguelikeGame::PLAYER_WEAPON_SLOTS, 0));
	screen.SetBeltSlots(Loadout(RoguelikeGame::QUICK_BELT_HOOKS, 0));

	EXPECT_EQ(screen.GetBeltSlotsShown(), RoguelikeGame::QUICK_BELT_HOOKS);

	sf::FloatRect lastWeapon = screen.GetWeaponSlotPanel(RoguelikeGame::PLAYER_WEAPON_SLOTS - 1).GetBounds();
	sf::FloatRect firstHook = screen.GetBeltSlotPanel(0).GetBounds();

	EXPECT_GT(firstHook.left, lastWeapon.left + lastWeapon.width) << "пояс налез на оружие";

	float insideGap = screen.GetWeaponSlotPanel(1).GetBounds().left
		- (screen.GetWeaponSlotPanel(0).GetBounds().left + lastWeapon.width);

	EXPECT_GT(firstHook.left - (lastWeapon.left + lastWeapon.width), insideGap)
		<< "между тройками зазора нет - читается как шесть одинаковых ячеек";
}

TEST_F(WeaponRowTest, AnEmptyHookKeepsItsKeyAndShowsNoCount)
{
	std::vector<SlotHudState> hooks(RoguelikeGame::QUICK_BELT_HOOKS);
	hooks[0].key = 4;
	hooks[1].key = 5;
	hooks[2].key = 6;

	screen.SetBeltSlots(hooks);

	EXPECT_EQ(screen.GetBeltSlotKey(0).GetText(), XYZEngine::FromUtf8("4"));
	EXPECT_FALSE(screen.GetBeltSlotCount(0).IsVisible()) << "у пустого крючка взялся счётчик";
	EXPECT_FALSE(screen.GetBeltSlotIcon(0).IsVisible());
}

TEST_F(WeaponRowTest, AHookWithZeroKeepsItsIconAndGoesDim)
{
	std::vector<SlotHudState> hooks(RoguelikeGame::QUICK_BELT_HOOKS);
	hooks[0].key = 4;
	hooks[0].isFilled = true;
	hooks[0].hasCount = true;
	hooks[0].count = 2;

	screen.SetBeltSlots(hooks);
	sf::Color plenty = screen.GetBeltSlotCount(0).GetColor();

	hooks[0].count = 0;
	hooks[0].isLow = true;
	screen.SetBeltSlots(hooks);

	EXPECT_EQ(screen.GetBeltSlotCount(0).GetText(), XYZEngine::FromUtf8("0")) << "крючок с нулём замолчал";
	EXPECT_NE(screen.GetBeltSlotCount(0).GetColor(), plenty) << "пустой крючок не отличить от полного";
}

TEST_F(WeaponRowTest, TheRowStaysOnScreenOnAnyResolution)
{
	for (const sf::Vector2f size : {sf::Vector2f{1280.f, 720.f}, sf::Vector2f{1920.f, 1080.f}, sf::Vector2f{800.f, 600.f}})
	{
		screen.Resize(size);
		screen.SetWeaponSlots(Loadout(RoguelikeGame::PLAYER_WEAPON_SLOTS, 0));

		for (int slot = 0; slot < RoguelikeGame::PLAYER_WEAPON_SLOTS; slot++)
		{
			sf::FloatRect cell = screen.GetWeaponSlotPanel(slot).GetBounds();

			EXPECT_GE(cell.left, 0.f) << "ряд уехал за левый край на " << size.x << "x" << size.y;
			EXPECT_LE(cell.left + cell.width, size.x) << "ряд уехал за правый край на " << size.x << "x" << size.y;
			EXPECT_GE(cell.top, 0.f) << "ряд уехал за верх на " << size.x << "x" << size.y;
			EXPECT_LE(cell.top + cell.height, size.y) << "ряд уехал за низ на " << size.x << "x" << size.y;
		}
	}
}
