#include "pch.h"
#include "GameSettings.h"
#include "HudScreen.h"
#include "RenderSystem.h"

using RoguelikeGame::AmmoHudState;
using RoguelikeGame::HudScreen;
using RoguelikeGame::VitalsHudState;

namespace
{
	class HudScreenTest : public ::testing::Test
	{
	protected:
		void SetUp() override { XYZEngine::RenderSystem::Instance()->HandleResize(1280, 720); }
		void TearDown() override { XYZEngine::RenderSystem::Instance()->HandleResize(1280, 720); }
	};
}

TEST_F(HudScreenTest, VitalsBarsFollowTheValues)
{
	HudScreen hud;
	hud.Resize({1280.f, 720.f});

	VitalsHudState state;
	state.healthPart = 0.75f;
	state.staminaPart = 0.4f;
	hud.SetVitals(state);

	EXPECT_FLOAT_EQ(hud.GetHealthBar().GetValue(), 0.75f);
	EXPECT_FLOAT_EQ(hud.GetStaminaBar().GetValue(), 0.4f);
	EXPECT_FLOAT_EQ(hud.GetHealthBar().GetFillBounds().width,
		RoguelikeGame::VITALS_HUD_WIDTH * 0.75f);
}

TEST_F(HudScreenTest, HealthBarChangesColorOnThresholds)
{
	HudScreen hud;
	hud.Resize({1280.f, 720.f});

	VitalsHudState state;
	state.healthPart = 1.f;
	hud.SetVitals(state);
	EXPECT_EQ(hud.GetHealthBar().GetFillColor(), RoguelikeGame::VITALS_HUD_HEALTH_COLOR);

	state.healthPart = 0.4f;
	hud.SetVitals(state);
	EXPECT_EQ(hud.GetHealthBar().GetFillColor(), RoguelikeGame::VITALS_HUD_HEALTH_LOW_COLOR);

	state.healthPart = 0.2f;
	hud.SetVitals(state);
	EXPECT_EQ(hud.GetHealthBar().GetFillColor(), RoguelikeGame::VITALS_HUD_HEALTH_CRITICAL_COLOR);
}

TEST_F(HudScreenTest, DeadPlayerShowsEmptyBarNotNegative)
{
	HudScreen hud;
	hud.Resize({1280.f, 720.f});

	VitalsHudState state;
	state.healthPart = -0.5f;
	hud.SetVitals(state);

	EXPECT_FLOAT_EQ(hud.GetHealthBar().GetValue(), 0.f);
	EXPECT_FLOAT_EQ(hud.GetHealthBar().GetFillBounds().width, 0.f);
}

TEST_F(HudScreenTest, ExhaustedStaminaIsGreyed)
{
	HudScreen hud;
	hud.Resize({1280.f, 720.f});

	VitalsHudState state;
	state.isExhausted = true;
	hud.SetVitals(state);

	EXPECT_EQ(hud.GetStaminaBar().GetFillColor(), RoguelikeGame::VITALS_HUD_STAMINA_EMPTY_COLOR);
}

TEST_F(HudScreenTest, BarsStayInTopLeftOnAnyResolution)
{
	HudScreen hud;

	hud.Resize({1280.f, 720.f});
	EXPECT_FLOAT_EQ(hud.GetHealthBar().GetBounds().left, RoguelikeGame::VITALS_HUD_MARGIN_X);
	EXPECT_FLOAT_EQ(hud.GetHealthBar().GetBounds().top, RoguelikeGame::VITALS_HUD_MARGIN_Y);

	hud.Resize({800.f, 600.f});
	EXPECT_FLOAT_EQ(hud.GetHealthBar().GetBounds().left, RoguelikeGame::VITALS_HUD_MARGIN_X);
	EXPECT_FLOAT_EQ(hud.GetHealthBar().GetBounds().top, RoguelikeGame::VITALS_HUD_MARGIN_Y);
}

TEST_F(HudScreenTest, AmmoLineHidesWithoutMagazine)
{
	HudScreen hud;
	hud.Resize({1280.f, 720.f});

	AmmoHudState state;
	state.weaponName = "Bat";
	state.hasMagazine = false;
	hud.SetAmmo(state);

	EXPECT_FALSE(hud.GetAmmoLabel().IsVisible());

	state.hasMagazine = true;
	state.inMagazine = 5;
	state.reserve = 30;
	hud.SetAmmo(state);

	EXPECT_TRUE(hud.GetAmmoLabel().IsVisible());
}

TEST_F(HudScreenTest, ArmorBarIsHiddenWithoutArmor)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	RoguelikeGame::VitalsHudState state;
	state.healthPart = 1.f;
	state.staminaPart = 1.f;
	state.armorPart = 0.f;
	screen.SetVitals(state);

	EXPECT_FALSE(screen.GetArmorBar().IsVisible());
}

TEST_F(HudScreenTest, ArmorBarFollowsTheStat)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	RoguelikeGame::VitalsHudState state;
	state.healthPart = 1.f;
	state.staminaPart = 1.f;
	state.armorPart = 0.5f;
	screen.SetVitals(state);

	EXPECT_TRUE(screen.GetArmorBar().IsVisible());
	EXPECT_FLOAT_EQ(screen.GetArmorBar().GetValue(), 0.5f);

	state.armorPart = 1.f;
	screen.SetVitals(state);
	EXPECT_FLOAT_EQ(screen.GetArmorBar().GetValue(), 1.f);
}
