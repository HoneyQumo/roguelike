#include "pch.h"
#include "FrameClock.h"

using XYZEngine::FrameClock;

namespace
{
	class FrameClockTest : public ::testing::Test
	{
	protected:
		FrameClock* clock = FrameClock::Instance();

		void SetUp() override { clock->Reset(); }
		void TearDown() override { clock->Reset(); }
	};
}

TEST(FrameClockTests, AdvanceCountsFramesAndTime)
{
	auto clock = FrameClock::Instance();
	clock->Reset();

	clock->Advance(0.5f);
	clock->Advance(0.25f);

	EXPECT_EQ(clock->GetFrame(), 2u);
	EXPECT_FLOAT_EQ(clock->GetDeltaTime(), 0.25f);
	EXPECT_FLOAT_EQ(clock->GetElapsedSeconds(), 0.75f);
}

TEST(FrameClockTests, ResetStartsFromZero)
{
	auto clock = FrameClock::Instance();
	clock->Advance(1.f);

	clock->Reset();

	EXPECT_EQ(clock->GetFrame(), 0u);
	EXPECT_FLOAT_EQ(clock->GetDeltaTime(), 0.f);
	EXPECT_FLOAT_EQ(clock->GetElapsedSeconds(), 0.f);
}

TEST_F(FrameClockTest, NormalTimeIsNotScaled)
{
	clock->Advance(0.016f);

	EXPECT_FLOAT_EQ(clock->GetTimeScale(), 1.f);
	EXPECT_FLOAT_EQ(clock->GetDeltaTime(), 0.016f);
	EXPECT_FLOAT_EQ(clock->GetUnscaledDeltaTime(), 0.016f);
}

TEST_F(FrameClockTest, HitStopFreezesGameTimeButNotRealTime)
{
	clock->HitStop(0.05f);

	clock->Advance(0.016f);

	EXPECT_FLOAT_EQ(clock->GetDeltaTime(), 0.f);
	EXPECT_FLOAT_EQ(clock->GetUnscaledDeltaTime(), 0.016f);
	EXPECT_FLOAT_EQ(clock->GetTimeScale(), 0.f);
}

TEST_F(FrameClockTest, HitStopEndsAndTimeScaleReturnsToOne)
{
	clock->HitStop(0.05f);

	for (int frame = 0; frame < 4; frame++)
	{
		clock->Advance(0.016f);
	}

	EXPECT_FLOAT_EQ(clock->GetTimeScale(), 1.f);
	EXPECT_FLOAT_EQ(clock->GetDeltaTime(), 0.016f);
}

TEST_F(FrameClockTest, RepeatedHitStopsDoNotPileUp)
{
	float frozenTime = 0.f;
	for (int hit = 0; hit < 5; hit++)
	{
		clock->HitStop(0.04f);
	}

	for (int frame = 0; frame < 10; frame++)
	{
		clock->Advance(0.016f);
		if (clock->GetTimeScale() == 0.f)
		{
			frozenTime += 0.016f;
		}
	}

	EXPECT_LE(frozenTime, 0.04f + 0.016f);
}

TEST_F(FrameClockTest, SlowMotionBlendsBackToNormalSpeed)
{
	clock->SlowMotion(0.4f, 0.1f, 0.2f);

	clock->Advance(0.016f);
	EXPECT_FLOAT_EQ(clock->GetTimeScale(), 0.4f);
	EXPECT_FLOAT_EQ(clock->GetDeltaTime(), 0.016f * 0.4f);

	for (int frame = 0; frame < 7; frame++)
	{
		clock->Advance(0.016f);
	}
	float blending = clock->GetTimeScale();
	EXPECT_GT(blending, 0.4f);
	EXPECT_LT(blending, 1.f);

	for (int frame = 0; frame < 20; frame++)
	{
		clock->Advance(0.016f);
	}

	EXPECT_FLOAT_EQ(clock->GetTimeScale(), 1.f);
}

TEST_F(FrameClockTest, HitStopWinsOverSlowMotion)
{
	clock->SlowMotion(0.5f, 0.5f, 0.2f);
	clock->HitStop(0.05f);

	clock->Advance(0.016f);

	EXPECT_FLOAT_EQ(clock->GetTimeScale(), 0.f);
}

TEST_F(FrameClockTest, StopTimeEffectsRestoresNormalSpeedAtOnce)
{
	clock->SlowMotion(0.2f, 1.f, 1.f);
	clock->Advance(0.016f);

	clock->StopTimeEffects();
	clock->Advance(0.016f);

	EXPECT_FLOAT_EQ(clock->GetTimeScale(), 1.f);
	EXPECT_FLOAT_EQ(clock->GetDeltaTime(), 0.016f);
}

TEST_F(FrameClockTest, ElapsedSecondsFollowGameTime)
{
	clock->SetTimeScale(0.5f);
	clock->Advance(0.1f);
	clock->Advance(0.1f);

	EXPECT_FLOAT_EQ(clock->GetElapsedSeconds(), 0.1f);
}
