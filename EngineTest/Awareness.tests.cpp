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
using RoguelikeGame::VisionBand;

namespace
{
	AwarenessSense Beside(float distance, float maxDistance = 400.f)
	{
		AwarenessSense sense;
		sense.band = VisionBand::Periphery;
		sense.distance = distance;
		sense.maxDistance = maxDistance;

		return sense;
	}

	AwarenessSense Behind(float distance, float maxDistance = 96.f)
	{
		AwarenessSense sense = Beside(distance, maxDistance);
		sense.band = VisionBand::Back;

		return sense;
	}

	AwarenessSense InFront()
	{
		AwarenessSense sense;
		sense.band = VisionBand::Focus;

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

	EXPECT_EQ(StateOf(Watch(rates, Beside(400.f), 0.5f)), AwarenessState::Calm);
	EXPECT_EQ(StateOf(Watch(rates, Beside(400.f), 1.2f)), AwarenessState::Alerted);
	EXPECT_EQ(StateOf(Watch(rates, Beside(400.f), 2.2f)), AwarenessState::Provoked);
}

TEST(AwarenessTest, CloserTargetIsNoticedSooner)
{
	AwarenessRates rates;

	EXPECT_GT(Watch(rates, Beside(0.f), 1.f), Watch(rates, Beside(400.f), 1.f));
}

TEST(AwarenessTest, MovingTargetIsNoticedSooner)
{
	AwarenessRates rates;
	AwarenessSense running = Beside(200.f);
	running.isTargetMoving = true;

	EXPECT_GT(Watch(rates, running, 1.f), Watch(rates, Beside(200.f), 1.f));
}

TEST(AwarenessTest, QuickEnemyNoticesSoonerThanSlowOne)
{
	AwarenessRates slow;
	slow.gain = 0.9f;
	AwarenessRates quick;
	quick.gain = 1.8f;

	EXPECT_GT(Watch(quick, Beside(200.f), 1.f), Watch(slow, Beside(200.f), 1.f));
}

TEST(AwarenessTest, OutOfSightTheLevelFallsBack)
{
	AwarenessRates rates;
	float level = Watch(rates, Beside(0.f), 2.f);
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

	EXPECT_FLOAT_EQ(NextAwareness(0.f, rates, Beside(0.f), 100.f), RoguelikeGame::AWARENESS_PROVOKE_AT);
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

	EXPECT_FLOAT_EQ(NextAwareness(0.5f, rates, Beside(0.f), 0.f), 0.5f);
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
	AwarenessSense close = Beside(0.f);
	AwarenessSense closeAndRunning = Beside(0.f);
	closeAndRunning.isTargetMoving = true;

	EXPECT_GT(GainFor(rates, close), GainFor(rates, Beside(400.f)));
	EXPECT_GT(GainFor(rates, closeAndRunning), GainFor(rates, close));
}

TEST(AwarenessTest, PartShowsHowFullTheScaleIs)
{
	EXPECT_FLOAT_EQ(AwarenessPart(0.f), 0.f);
	EXPECT_FLOAT_EQ(AwarenessPart(RoguelikeGame::AWARENESS_PROVOKE_AT), 1.f);
	EXPECT_FLOAT_EQ(AwarenessPart(RoguelikeGame::AWARENESS_PROVOKE_AT * 2.f), 1.f);
	EXPECT_GT(AwarenessPart(RoguelikeGame::AWARENESS_ALERT_AT), 0.f);
}

TEST(AwarenessTest, TargetInFrontIsSpottedAtOnce)
{
	AwarenessRates rates;

	EXPECT_FLOAT_EQ(NextAwareness(0.f, rates, InFront(), 0.01f), RoguelikeGame::AWARENESS_PROVOKE_AT);
	EXPECT_EQ(StateOf(NextAwareness(0.f, rates, InFront(), 0.01f)), AwarenessState::Provoked);
}

TEST(AwarenessTest, BehindTheBackIsNoticedSlowerThanFromTheSide)
{
	AwarenessRates rates;

	EXPECT_LT(GainFor(rates, Behind(0.f)), GainFor(rates, Beside(0.f)));
}

TEST(AwarenessTest, NobodySeenMeansNoGainAtAll)
{
	AwarenessRates rates;

	EXPECT_FLOAT_EQ(GainFor(rates, Hidden()), 0.f);
}

TEST(AwarenessTest, BehindTheBackStillFillsTheScale)
{
	AwarenessRates rates;

	EXPECT_EQ(StateOf(Watch(rates, Behind(10.f), 3.f)), AwarenessState::Provoked);
}
