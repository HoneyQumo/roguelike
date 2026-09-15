#include "pch.h"
#include "Awareness.h"

using RoguelikeGame::AwarenessPart;
using RoguelikeGame::AwarenessRates;
using RoguelikeGame::AwarenessSense;
using RoguelikeGame::AwarenessState;
using RoguelikeGame::GainFor;
using RoguelikeGame::Nearness;
using RoguelikeGame::NextAwareness;
using RoguelikeGame::StateOf;

namespace
{
	AwarenessSense Seen(float distance, float maxDistance = 400.f)
	{
		AwarenessSense sense;
		sense.isVisible = true;
		sense.distance = distance;
		sense.maxDistance = maxDistance;

		return sense;
	}

	AwarenessSense Hidden()
	{
		return AwarenessSense();
	}

	float Watch(const AwarenessRates& rates, AwarenessSense sense, float seconds, float step = 0.05f)
	{
		float level = 0.f;
		for (float passed = 0.f; passed < seconds; passed += step)
		{
			level = NextAwareness(level, rates, sense, step);
		}

		return level;
	}
}

TEST(AwarenessTest, NobodyInSightMeansCalm)
{
	EXPECT_EQ(StateOf(0.f), AwarenessState::Calm);
}

TEST(AwarenessTest, WatchingLongEnoughGoesThroughAlertToCombat)
{
	AwarenessRates rates;

	EXPECT_EQ(StateOf(Watch(rates, Seen(400.f), 0.5f)), AwarenessState::Calm);
	EXPECT_EQ(StateOf(Watch(rates, Seen(400.f), 1.2f)), AwarenessState::Alerted);
	EXPECT_EQ(StateOf(Watch(rates, Seen(400.f), 2.2f)), AwarenessState::Provoked);
}

TEST(AwarenessTest, CloserTargetIsNoticedSooner)
{
	AwarenessRates rates;

	EXPECT_GT(Watch(rates, Seen(0.f), 1.f), Watch(rates, Seen(400.f), 1.f));
}

TEST(AwarenessTest, MovingTargetIsNoticedSooner)
{
	AwarenessRates rates;
	AwarenessSense running = Seen(200.f);
	running.isTargetMoving = true;

	EXPECT_GT(Watch(rates, running, 1.f), Watch(rates, Seen(200.f), 1.f));
}

TEST(AwarenessTest, QuickEnemyNoticesSoonerThanSlowOne)
{
	AwarenessRates slow;
	slow.gain = 0.9f;
	AwarenessRates quick;
	quick.gain = 1.8f;

	EXPECT_GT(Watch(quick, Seen(200.f), 1.f), Watch(slow, Seen(200.f), 1.f));
}

TEST(AwarenessTest, OutOfSightTheLevelFallsBack)
{
	AwarenessRates rates;
	float level = Watch(rates, Seen(0.f), 2.f);
	ASSERT_GT(level, 0.f);

	level = NextAwareness(level, rates, Hidden(), 1.f);

	EXPECT_LT(level, RoguelikeGame::AWARENESS_PROVOKE_AT);
}

TEST(AwarenessTest, LevelNeverFallsBelowNothing)
{
	AwarenessRates rates;

	EXPECT_FLOAT_EQ(NextAwareness(0.f, rates, Hidden(), 10.f), 0.f);
}

TEST(AwarenessTest, LevelNeverClimbsAboveCombat)
{
	AwarenessRates rates;

	EXPECT_FLOAT_EQ(NextAwareness(0.f, rates, Seen(0.f), 100.f), RoguelikeGame::AWARENESS_PROVOKE_AT);
}

TEST(AwarenessTest, EnemyThatAlreadySearchesDoesNotCoolDown)
{
	AwarenessRates rates;
	AwarenessSense lost = Hidden();
	lost.keepsMemory = true;

	EXPECT_FLOAT_EQ(NextAwareness(RoguelikeGame::AWARENESS_PROVOKE_AT, rates, lost, 5.f), RoguelikeGame::AWARENESS_PROVOKE_AT);
}

TEST(AwarenessTest, NoTimeMeansNoChange)
{
	AwarenessRates rates;

	EXPECT_FLOAT_EQ(NextAwareness(0.5f, rates, Seen(0.f), 0.f), 0.5f);
}

TEST(AwarenessTest, NearnessIsOneUpCloseAndNothingFarAway)
{
	EXPECT_FLOAT_EQ(Nearness(0.f, 400.f), 1.f);
	EXPECT_FLOAT_EQ(Nearness(400.f, 400.f), 0.f);
	EXPECT_FLOAT_EQ(Nearness(600.f, 400.f), 0.f);
	EXPECT_FLOAT_EQ(Nearness(200.f, 400.f), 0.5f);
}

TEST(AwarenessTest, WithoutARadiusNothingIsNear)
{
	EXPECT_FLOAT_EQ(Nearness(10.f, 0.f), 0.f);
}

TEST(AwarenessTest, GainGrowsWithBothReasons)
{
	AwarenessRates rates;
	AwarenessSense close = Seen(0.f);
	AwarenessSense closeAndRunning = Seen(0.f);
	closeAndRunning.isTargetMoving = true;

	EXPECT_GT(GainFor(rates, close), GainFor(rates, Seen(400.f)));
	EXPECT_GT(GainFor(rates, closeAndRunning), GainFor(rates, close));
}

TEST(AwarenessTest, PartShowsHowFullTheScaleIs)
{
	EXPECT_FLOAT_EQ(AwarenessPart(0.f), 0.f);
	EXPECT_FLOAT_EQ(AwarenessPart(RoguelikeGame::AWARENESS_PROVOKE_AT), 1.f);
	EXPECT_FLOAT_EQ(AwarenessPart(RoguelikeGame::AWARENESS_PROVOKE_AT * 2.f), 1.f);
	EXPECT_GT(AwarenessPart(RoguelikeGame::AWARENESS_ALERT_AT), 0.f);
}
