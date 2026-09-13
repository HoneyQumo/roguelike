#include "pch.h"
#include "FadeScreen.h"

using RoguelikeGame::FadeScreen;

namespace
{
	void Advance(FadeScreen& fade, float seconds, float step = 0.05f)
	{
		for (float passed = 0.f; passed < seconds; passed += step)
		{
			fade.Update(step);
		}
	}
}

TEST(FadeScreenTest, NewScreenIsClearAndHidden)
{
	FadeScreen fade;

	EXPECT_TRUE(fade.IsClear());
	EXPECT_FALSE(fade.IsBlackout());
	EXPECT_FALSE(fade.IsVisible());
}

TEST(FadeScreenTest, FadeOutReachesBlackoutAndStays)
{
	FadeScreen fade;
	fade.FadeOut(0.3f);

	Advance(fade, 0.2f);
	EXPECT_FALSE(fade.IsBlackout());

	Advance(fade, 0.2f);
	EXPECT_TRUE(fade.IsBlackout());
	EXPECT_TRUE(fade.IsVisible());

	Advance(fade, 1.f);
	EXPECT_TRUE(fade.IsBlackout());
}

TEST(FadeScreenTest, FadeInClearsTheScreenAndHidesIt)
{
	FadeScreen fade;
	fade.Blackout();
	ASSERT_TRUE(fade.IsBlackout());

	fade.FadeIn(0.3f);
	Advance(fade, 0.5f);

	EXPECT_TRUE(fade.IsClear());
	EXPECT_FALSE(fade.IsVisible());
}

TEST(FadeScreenTest, BlackoutIsInstant)
{
	FadeScreen fade;

	fade.Blackout();

	EXPECT_TRUE(fade.IsBlackout());
	EXPECT_TRUE(fade.IsVisible());
}

TEST(FadeScreenTest, FadeIsMonotonic)
{
	FadeScreen fade;
	fade.FadeOut(0.4f);

	float previous = fade.GetPart();
	for (int step = 0; step < 6; step++)
	{
		fade.Update(0.05f);
		EXPECT_GE(fade.GetPart(), previous);
		previous = fade.GetPart();
	}
}

TEST(FadeScreenTest, ZeroDurationSwitchesAtOnce)
{
	FadeScreen fade;

	fade.FadeOut(0.f);

	EXPECT_TRUE(fade.IsBlackout());
}

TEST(FadeScreenTest, FadeCanBeReversedMidway)
{
	FadeScreen fade;
	fade.FadeOut(0.4f);
	Advance(fade, 0.2f);
	float midway = fade.GetPart();
	ASSERT_GT(midway, 0.f);
	ASSERT_LT(midway, 1.f);

	fade.FadeIn(0.4f);
	Advance(fade, 0.5f);

	EXPECT_TRUE(fade.IsClear());
}

TEST(FadeScreenTest, ArrivalBlackIsNotTreatedAsDeparture)
{
	FadeScreen fade;
	fade.Blackout();
	fade.FadeIn(0.4f);

	EXPECT_TRUE(fade.IsBlackout());
	EXPECT_FALSE(fade.IsCovered());
}

TEST(FadeScreenTest, DepartureIsCoveredOnlyWhenFullyBlack)
{
	FadeScreen fade;
	fade.FadeOut(0.4f);

	EXPECT_FALSE(fade.IsCovered());

	Advance(fade, 0.5f);

	EXPECT_TRUE(fade.IsCovered());
}
