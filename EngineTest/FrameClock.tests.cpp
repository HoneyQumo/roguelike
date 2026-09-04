#include "pch.h"
#include "FrameClock.h"

using XYZEngine::FrameClock;

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
