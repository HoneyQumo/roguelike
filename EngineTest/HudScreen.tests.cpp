#include "pch.h"
#include "GameSettings.h"
#include "HudScreen.h"
#include "RenderSystem.h"
#include <TextUtils.h>

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

namespace
{
	RoguelikeGame::WaveHudState RunningWave(int current, int total, int left)
	{
		RoguelikeGame::WaveHudState state;
		state.isRunning = true;
		state.current = current;
		state.total = total;
		state.left = left;

		return state;
	}

	std::string TextOf(const XYZEngine::UiLabel& label)
	{
		std::basic_string<sf::Uint8> utf8 = label.GetText().toUtf8();

		return std::string(utf8.begin(), utf8.end());
	}
}

TEST_F(HudScreenTest, WithoutWavesThePanelIsNotOnScreen)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetWaves({});

	EXPECT_FALSE(screen.IsWavePanelShown());
}

TEST_F(HudScreenTest, TheWavePanelShowsWhichWaveItIs)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetWaves(RunningWave(3, 12, 4));

	EXPECT_TRUE(screen.IsWavePanelShown());
	EXPECT_EQ(TextOf(screen.GetWaveLabel()), u8"ВОЛНА 3 / 12") << "the title does not say where the player is";
}

TEST_F(HudScreenTest, TheCounterShowsWhatIsLeftToKill)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetWaves(RunningWave(3, 12, 4));

	EXPECT_EQ(TextOf(screen.GetWaveCountLabel()), u8"Осталось: 4");
}

TEST_F(HudScreenTest, TheBarFillsUpAsTheSiegeGoesOn)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetWaves(RunningWave(3, 12, 4));
	float early = screen.GetWaveBar().GetValue();

	screen.SetWaves(RunningWave(11, 12, 2));

	EXPECT_GT(screen.GetWaveBar().GetValue(), early) << "the bar does not move towards the end";
	EXPECT_FLOAT_EQ(screen.GetWaveBar().GetValue(), 11.f / 12.f);
}

TEST_F(HudScreenTest, ALullSaysHowLongTheBreathIs)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	RoguelikeGame::WaveHudState state = RunningWave(3, 12, 0);
	state.isPause = true;
	state.nextIn = 4.2f;
	screen.SetWaves(state);

	EXPECT_TRUE(screen.IsWavePanelShown());
	EXPECT_EQ(TextOf(screen.GetWaveLabel()), u8"Затишье");
	EXPECT_EQ(TextOf(screen.GetWaveCountLabel()), u8"Следующая волна через 5");
}

TEST_F(HudScreenTest, TheLullIsToldApartByColour)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetWaves(RunningWave(3, 12, 4));
	EXPECT_EQ(screen.GetWaveBar().GetFillColor(), RoguelikeGame::WAVE_HUD_BAR_COLOR);

	RoguelikeGame::WaveHudState lull = RunningWave(3, 12, 0);
	lull.isPause = true;
	screen.SetWaves(lull);

	EXPECT_EQ(screen.GetWaveBar().GetFillColor(), RoguelikeGame::WAVE_HUD_CALM_COLOR);
}

TEST_F(HudScreenTest, WhenTheSiegeIsOverThePanelGoesAway)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetWaves(RunningWave(12, 12, 0));
	ASSERT_TRUE(screen.IsWavePanelShown());

	screen.SetWaves({});

	EXPECT_FALSE(screen.IsWavePanelShown());
}

TEST_F(HudScreenTest, TheWavePanelKeepsClearOfTheVitals)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetWaves(RunningWave(1, 12, 3));

	sf::FloatRect panel = screen.GetWaveBar().GetBounds();
	sf::FloatRect vitals = screen.GetHealthBar().GetBounds();

	EXPECT_FALSE(panel.intersects(vitals)) << "the wave panel sits on top of the health bar";
}

namespace
{
	RoguelikeGame::ChaseHudState Chasing(float progress, bool isClose)
	{
		RoguelikeGame::ChaseHudState state;
		state.isRunning = true;
		state.progress = progress;
		state.isClose = isClose;

		return state;
	}
}

TEST_F(HudScreenTest, WithoutAChaseThePanelIsNotOnScreen)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetChase({});

	EXPECT_FALSE(screen.IsChasePanelShown());
}

TEST_F(HudScreenTest, TheChaseBarShowsHowMuchOfTheBridgeIsBehind)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetChase(Chasing(0.25f, false));
	EXPECT_TRUE(screen.IsChasePanelShown());
	EXPECT_FLOAT_EQ(screen.GetChaseBar().GetValue(), 0.25f);

	screen.SetChase(Chasing(0.8f, false));
	EXPECT_FLOAT_EQ(screen.GetChaseBar().GetValue(), 0.8f);
}

TEST_F(HudScreenTest, TheChaseBarNeverRunsPastItsEnds)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetChase(Chasing(-1.f, false));
	EXPECT_FLOAT_EQ(screen.GetChaseBar().GetValue(), 0.f);

	screen.SetChase(Chasing(3.f, false));
	EXPECT_FLOAT_EQ(screen.GetChaseBar().GetValue(), 1.f);
}

TEST_F(HudScreenTest, TheRunnerIsToldWhetherTheChaseIsOnHisHeels)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetChase(Chasing(0.5f, true));
	EXPECT_EQ(TextOf(screen.GetChaseLabel()), u8"Погоня близко");
	EXPECT_EQ(screen.GetChaseBar().GetFillColor(), RoguelikeGame::CHASE_HUD_CLOSE_COLOR);

	screen.SetChase(Chasing(0.5f, false));
	EXPECT_EQ(TextOf(screen.GetChaseLabel()), u8"Оторвался");
	EXPECT_EQ(screen.GetChaseBar().GetFillColor(), RoguelikeGame::CHASE_HUD_AWAY_COLOR);
}

TEST_F(HudScreenTest, TheChasePanelKeepsClearOfTheVitals)
{
	HudScreen screen;
	screen.Resize({1280.f, 720.f});

	screen.SetChase(Chasing(0.1f, true));

	EXPECT_FALSE(screen.GetChaseBar().GetBounds().intersects(screen.GetHealthBar().GetBounds()))
		<< "the chase bar sits on top of the health bar";
}
