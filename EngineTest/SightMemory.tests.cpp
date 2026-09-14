#include "pch.h"
#include "SightMemory.h"

using RoguelikeGame::Alarm;
using RoguelikeGame::Fade;
using RoguelikeGame::Forget;
using RoguelikeGame::IsSearching;
using RoguelikeGame::Remember;
using RoguelikeGame::SightMemory;

TEST(SightMemoryTest, FreshEnemyRemembersNothing)
{
	SightMemory memory;

	EXPECT_FALSE(IsSearching(memory));
	EXPECT_FALSE(memory.hasPoint);
}

TEST(SightMemoryTest, SeeingTheTargetStartsTheSearchWindow)
{
	SightMemory memory = Remember(SightMemory(), 3.f);

	EXPECT_TRUE(IsSearching(memory));
	EXPECT_TRUE(memory.hasPoint);
	EXPECT_FLOAT_EQ(memory.alertLeft, 3.f);
}

TEST(SightMemoryTest, LostTargetIsSearchedUntilTheTimeRunsOut)
{
	SightMemory memory = Remember(SightMemory(), 3.f);

	memory = Fade(memory, 2.f);
	EXPECT_TRUE(IsSearching(memory));
	EXPECT_TRUE(memory.hasPoint);

	memory = Fade(memory, 2.f);
	EXPECT_FALSE(IsSearching(memory));
	EXPECT_FALSE(memory.hasPoint);
}

TEST(SightMemoryTest, SeeingAgainRefillsTheWindow)
{
	SightMemory memory = Remember(SightMemory(), 3.f);
	memory = Fade(memory, 2.5f);
	memory = Remember(memory, 3.f);

	EXPECT_FLOAT_EQ(memory.alertLeft, 3.f);
}

TEST(SightMemoryTest, ShorterRefreshDoesNotCutALongerAlarm)
{
	SightMemory memory = Alarm(SightMemory(), 8.f);
	memory = Remember(memory, 3.f);

	EXPECT_FLOAT_EQ(memory.alertLeft, 8.f);
	EXPECT_TRUE(memory.hasPoint);
}

TEST(SightMemoryTest, ArrivingAtAnEmptyPointDropsItButKeepsSearching)
{
	SightMemory memory = Remember(SightMemory(), 3.f);
	memory = Forget(memory);

	EXPECT_FALSE(memory.hasPoint);
	EXPECT_TRUE(IsSearching(memory));
}

TEST(SightMemoryTest, AlarmWithoutAPointRaisesOnlyTheTimer)
{
	SightMemory memory = Alarm(SightMemory(), 5.f);

	EXPECT_TRUE(IsSearching(memory));
	EXPECT_FALSE(memory.hasPoint);
}

TEST(SightMemoryTest, AlarmKeepsAPointItAlreadyHad)
{
	SightMemory memory = Remember(SightMemory(), 3.f);
	memory = Alarm(memory, 5.f);

	EXPECT_TRUE(memory.hasPoint);
	EXPECT_FLOAT_EQ(memory.alertLeft, 5.f);
}

TEST(SightMemoryTest, TimeoutForgetsThePointToo)
{
	SightMemory memory = Remember(SightMemory(), 1.f);
	memory = Fade(memory, 1.f);

	EXPECT_FALSE(IsSearching(memory));
	EXPECT_FALSE(memory.hasPoint);
}

TEST(SightMemoryTest, EnemyWithoutSearchTimeNeverSearches)
{
	SightMemory memory = Remember(SightMemory(), 0.f);

	EXPECT_FALSE(IsSearching(memory));
	EXPECT_FALSE(memory.hasPoint);
}

TEST(SightMemoryTest, FadeDoesNotGoBelowZero)
{
	SightMemory memory = Remember(SightMemory(), 1.f);
	memory = Fade(memory, 10.f);

	EXPECT_FLOAT_EQ(memory.alertLeft, 0.f);
}
