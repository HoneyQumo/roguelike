#include "pch.h"
#include "SightMemory.h"

using RoguelikeGame::Alarm;
using RoguelikeGame::Forget;
using RoguelikeGame::GiveUp;
using RoguelikeGame::IsSearching;
using RoguelikeGame::Remember;
using RoguelikeGame::SightMemory;
using RoguelikeGame::Tick;

namespace
{
	constexpr float TRAVEL = 12.f;
	constexpr bool ON_THE_WAY = true;
	constexpr bool STANDING = false;
}

TEST(SightMemoryTest, FreshEnemyRemembersNothing)
{
	SightMemory memory;

	EXPECT_FALSE(IsSearching(memory));
	EXPECT_FALSE(memory.hasPoint);
}

TEST(SightMemoryTest, SeeingTheTargetStartsTheSearchWindow)
{
	SightMemory memory = Remember(SightMemory(), 3.f, TRAVEL);

	EXPECT_TRUE(IsSearching(memory));
	EXPECT_TRUE(memory.hasPoint);
	EXPECT_FLOAT_EQ(memory.alertLeft, 3.f);
	EXPECT_FLOAT_EQ(memory.travelLeft, TRAVEL);
}

TEST(SightMemoryTest, LostTargetIsSearchedUntilTheTimeRunsOut)
{
	SightMemory memory = Remember(SightMemory(), 3.f, TRAVEL);

	memory = Tick(memory, 2.f, STANDING);
	EXPECT_TRUE(IsSearching(memory));
	EXPECT_TRUE(memory.hasPoint);

	memory = Tick(memory, 2.f, STANDING);
	EXPECT_FALSE(IsSearching(memory));
	EXPECT_FALSE(memory.hasPoint);
}

TEST(SightMemoryTest, SeeingAgainRefillsTheWindow)
{
	SightMemory memory = Remember(SightMemory(), 3.f, TRAVEL);
	memory = Tick(memory, 2.5f, STANDING);
	memory = Remember(memory, 3.f, TRAVEL);

	EXPECT_FLOAT_EQ(memory.alertLeft, 3.f);
}

TEST(SightMemoryTest, ShorterRefreshDoesNotCutALongerAlarm)
{
	SightMemory memory = Alarm(SightMemory(), 8.f);
	memory = Remember(memory, 3.f, TRAVEL);

	EXPECT_FLOAT_EQ(memory.alertLeft, 8.f);
	EXPECT_TRUE(memory.hasPoint);
}

TEST(SightMemoryTest, ArrivingAtAnEmptyPointDropsItButKeepsSearching)
{
	SightMemory memory = Remember(SightMemory(), 3.f, TRAVEL);
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
	SightMemory memory = Remember(SightMemory(), 3.f, TRAVEL);
	memory = Alarm(memory, 5.f);

	EXPECT_TRUE(memory.hasPoint);
	EXPECT_FLOAT_EQ(memory.alertLeft, 5.f);
}

TEST(SightMemoryTest, TimeoutForgetsThePointToo)
{
	SightMemory memory = Remember(SightMemory(), 1.f, TRAVEL);
	memory = Tick(memory, 1.f, STANDING);

	EXPECT_FALSE(IsSearching(memory));
	EXPECT_FALSE(memory.hasPoint);
}

TEST(SightMemoryTest, EnemyWithoutSearchTimeNeverSearches)
{
	SightMemory memory = Remember(SightMemory(), 0.f, TRAVEL);

	EXPECT_FALSE(IsSearching(memory));
	EXPECT_FALSE(memory.hasPoint);
}

TEST(SightMemoryTest, TickDoesNotGoBelowZero)
{
	SightMemory memory = Remember(SightMemory(), 1.f, TRAVEL);
	memory = Tick(memory, 10.f, STANDING);

	EXPECT_FLOAT_EQ(memory.alertLeft, 0.f);
}

TEST(SightMemoryTest, GivingUpLeavesOnlyAShortLookAround)
{
	SightMemory memory = Remember(SightMemory(), 8.f, TRAVEL);
	memory = GiveUp(memory, 1.5f);

	EXPECT_FALSE(memory.hasPoint);
	EXPECT_FLOAT_EQ(memory.alertLeft, 1.5f);
	EXPECT_FLOAT_EQ(memory.travelLeft, 0.f);
	EXPECT_TRUE(IsSearching(memory));
}

TEST(SightMemoryTest, GivingUpDoesNotStretchAShortAlarm)
{
	SightMemory memory = Remember(SightMemory(), 0.5f, TRAVEL);
	memory = GiveUp(memory, 1.5f);

	EXPECT_FLOAT_EQ(memory.alertLeft, 0.5f);
}

TEST(SightMemoryTest, GivingUpWithoutLookAroundEndsTheSearch)
{
	SightMemory memory = Remember(SightMemory(), 8.f, TRAVEL);
	memory = GiveUp(memory, 0.f);

	EXPECT_FALSE(IsSearching(memory));
	EXPECT_FALSE(memory.hasPoint);
}

TEST(SightMemoryTest, TheWayToThePointDoesNotEatTheAlarm)
{
	SightMemory memory = Remember(SightMemory(), 3.f, TRAVEL);

	memory = Tick(memory, 2.5f, ON_THE_WAY);

	EXPECT_FLOAT_EQ(memory.alertLeft, 3.f) << "the alarm ran out on the way to the point";
	EXPECT_FLOAT_EQ(memory.travelLeft, TRAVEL - 2.5f);
	EXPECT_TRUE(memory.hasPoint);
}

TEST(SightMemoryTest, AWalkThatNeverEndsIsGivenUp)
{
	SightMemory memory = Remember(SightMemory(), 3.f, 2.f);

	memory = Tick(memory, 2.5f, ON_THE_WAY);

	EXPECT_FALSE(memory.hasPoint) << "the enemy walks towards an unreachable point forever";
	EXPECT_FLOAT_EQ(memory.travelLeft, 0.f);
	EXPECT_TRUE(IsSearching(memory)) << "giving up the road is not a reason to calm down";
}

TEST(SightMemoryTest, TheAlarmStartsFadingOnlyOnArrival)
{
	SightMemory memory = Remember(SightMemory(), 3.f, TRAVEL);

	memory = Tick(memory, 2.f, ON_THE_WAY);
	memory = Tick(memory, 2.f, STANDING);

	EXPECT_FLOAT_EQ(memory.alertLeft, 1.f);
}

TEST(SightMemoryTest, AFreshAlarmGivesTheRoadBack)
{
	SightMemory memory = Remember(SightMemory(), 3.f, TRAVEL);
	memory = Tick(memory, 5.f, ON_THE_WAY);

	memory = Remember(memory, 3.f, TRAVEL);

	EXPECT_FLOAT_EQ(memory.travelLeft, TRAVEL);
}

TEST(SightMemoryTest, StandingStillWithAPointStillCalmsDown)
{
	SightMemory memory = Remember(SightMemory(), 1.f, TRAVEL);

	memory = Tick(memory, 1.f, STANDING);

	EXPECT_FALSE(IsSearching(memory));
	EXPECT_FLOAT_EQ(memory.travelLeft, 0.f);
}
