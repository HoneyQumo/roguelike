#include "pch.h"
#include "ProgressWatch.h"

using RoguelikeGame::IsBlocked;
using RoguelikeGame::ProgressWatch;
using RoguelikeGame::Unblock;
using RoguelikeGame::WatchProgress;

namespace
{
	constexpr float WINDOW = 0.4f;
	constexpr float BLOCKED = 2.f;
	constexpr float SHARE = 0.4f;
	constexpr float STEP = 0.1f;

	ProgressWatch Walk(ProgressWatch watch, float movedPerStep, float wantedPerStep, int steps)
	{
		for (int step = 0; step < steps; step++)
		{
			watch = WatchProgress(watch, movedPerStep, wantedPerStep, STEP, WINDOW, BLOCKED, SHARE);
		}

		return watch;
	}
}

TEST(ProgressWatchTest, FreshWatchIsNotBlocked)
{
	EXPECT_FALSE(IsBlocked(ProgressWatch()));
}

TEST(ProgressWatchTest, FullSpeedIsNotBlocked)
{
	EXPECT_FALSE(IsBlocked(Walk(ProgressWatch(), 10.f, 10.f, 20)));
}

TEST(ProgressWatchTest, CrawlingAlongAWallIsBlocked)
{
	EXPECT_TRUE(IsBlocked(Walk(ProgressWatch(), 1.f, 10.f, 8)));
}

TEST(ProgressWatchTest, StandingStillIsBlocked)
{
	EXPECT_TRUE(IsBlocked(Walk(ProgressWatch(), 0.f, 10.f, 8)));
}

TEST(ProgressWatchTest, SlowButGoodEnoughIsNotBlocked)
{
	EXPECT_FALSE(IsBlocked(Walk(ProgressWatch(), 5.f, 10.f, 20)));
}

TEST(ProgressWatchTest, StandingWithoutWantingToMoveIsFine)
{
	EXPECT_FALSE(IsBlocked(Walk(ProgressWatch(), 0.f, 0.f, 20)));
}

TEST(ProgressWatchTest, NoVerdictBeforeTheWindowEnds)
{
	ProgressWatch watch = Walk(ProgressWatch(), 0.f, 10.f, 3);

	EXPECT_FALSE(IsBlocked(watch));
}

TEST(ProgressWatchTest, BlockStateFadesAfterItsTime)
{
	ProgressWatch watch = Walk(ProgressWatch(), 0.f, 10.f, 8);
	ASSERT_TRUE(IsBlocked(watch));

	watch = Walk(watch, 10.f, 10.f, 25);

	EXPECT_FALSE(IsBlocked(watch));
}

TEST(ProgressWatchTest, UnblockForgetsEverything)
{
	ProgressWatch watch = Walk(ProgressWatch(), 0.f, 10.f, 8);
	ASSERT_TRUE(IsBlocked(watch));

	EXPECT_FALSE(IsBlocked(Unblock(watch)));
}
