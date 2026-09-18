#include "pch.h"
#include "GameSettings.h"
#include "HudScreen.h"
#include <RenderSystem.h>
#include <TextUtils.h>

using RoguelikeGame::HudScreen;
using RoguelikeGame::WeaponSlotHudState;

namespace
{
	std::vector<WeaponSlotHudState> Loadout(int count, int current, bool isArmed = true)
	{
		std::vector<WeaponSlotHudState> slots;

		for (int slot = 0; slot < count; slot++)
		{
			WeaponSlotHudState shown;
			shown.hasWeapon = isArmed || slot == current;
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
	std::vector<WeaponSlotHudState> slots = Loadout(RoguelikeGame::PLAYER_WEAPON_SLOTS, 2, false);
	screen.SetWeaponSlots(slots);

	EXPECT_FALSE(screen.GetWeaponSlotIcon(0).IsVisible()) << "в пустом слоте рисуется оружие";
	EXPECT_EQ(screen.GetWeaponSlotKey(0).GetText(), XYZEngine::FromUtf8("1")) << "пустой слот потерял свою цифру";
	EXPECT_NE(screen.GetWeaponSlotPanel(0).GetShape().getFillColor(), RoguelikeGame::INVENTORY_SLOT_FILLED_COLOR);
}

TEST_F(WeaponRowTest, TheKeyComesFromTheStateAndNotFromTheCellNumber)
{
	std::vector<WeaponSlotHudState> slots = Loadout(2, 0);
	slots[1].key = 7;

	screen.SetWeaponSlots(slots);

	EXPECT_EQ(screen.GetWeaponSlotKey(1).GetText(), XYZEngine::FromUtf8("7")) << "подпись написана руками, а не взята из привязки";
}

TEST_F(WeaponRowTest, TheRowKeepsClearOfTheAmmoBlockAndTheVitals)
{
	screen.SetWeaponSlots(Loadout(RoguelikeGame::PLAYER_WEAPON_SLOTS, 0));
	screen.SetAmmo({"AK", 30, 90, true, false, false});
	screen.SetVitals({1.f, 1.f, 0.f, false});

	for (int slot = 0; slot < RoguelikeGame::PLAYER_WEAPON_SLOTS; slot++)
	{
		sf::FloatRect cell = screen.GetWeaponSlotPanel(slot).GetBounds();

		EXPECT_FALSE(cell.intersects(screen.GetNameLabel().GetBounds())) << "ряд наехал на имя оружия, слот " << slot;
		EXPECT_FALSE(cell.intersects(screen.GetAmmoLabel().GetBounds())) << "ряд наехал на патроны, слот " << slot;
		EXPECT_FALSE(cell.intersects(screen.GetHealthBar().GetBounds())) << "ряд наехал на полосу жизни, слот " << slot;
		EXPECT_FALSE(cell.intersects(screen.GetPromptLabel().GetBounds())) << "ряд наехал на подсказку, слот " << slot;
	}
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
