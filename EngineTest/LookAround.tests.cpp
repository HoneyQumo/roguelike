#include "pch.h"
#include "LookAround.h"

using RoguelikeGame::IsLookDone;
using RoguelikeGame::LookAngleAt;
using RoguelikeGame::LookPlan;

namespace
{
	LookPlan Plan(float base = 90.f, float sweep = 40.f, float duration = 3.f)
	{
		LookPlan plan;
		plan.baseAngle = base;
		plan.halfSweep = sweep;
		plan.duration = duration;

		return plan;
	}
}

TEST(LookAroundTest, LookStartsAtTheWayItCame)
{
	EXPECT_FLOAT_EQ(LookAngleAt(Plan(), 0.f), 90.f);
}

TEST(LookAroundTest, FirstTurnGoesOneWay)
{
	EXPECT_FLOAT_EQ(LookAngleAt(Plan(), 0.5f), 110.f);
	EXPECT_FLOAT_EQ(LookAngleAt(Plan(), 1.f), 130.f);
}

TEST(LookAroundTest, ThenItSweepsToTheOtherSide)
{
	EXPECT_FLOAT_EQ(LookAngleAt(Plan(), 2.f), 50.f);
}

TEST(LookAroundTest, AndComesBackToTheWayItCame)
{
	EXPECT_FLOAT_EQ(LookAngleAt(Plan(), 3.f), 90.f);
}

TEST(LookAroundTest, SweepPassesThroughTheMiddle)
{
	EXPECT_FLOAT_EQ(LookAngleAt(Plan(), 1.5f), 90.f);
}

TEST(LookAroundTest, WiderSweepLooksFurtherAside)
{
	EXPECT_GT(LookAngleAt(Plan(90.f, 80.f), 1.f), LookAngleAt(Plan(90.f, 40.f), 1.f));
}

TEST(LookAroundTest, LookEndsWithItsTime)
{
	EXPECT_FALSE(IsLookDone(Plan(), 2.9f));
	EXPECT_TRUE(IsLookDone(Plan(), 3.f));
	EXPECT_TRUE(IsLookDone(Plan(), 10.f));
}

TEST(LookAroundTest, EnemyWithoutLookTimeDoesNotStop)
{
	LookPlan plan = Plan(90.f, 40.f, 0.f);

	EXPECT_TRUE(IsLookDone(plan, 0.f));
	EXPECT_FLOAT_EQ(LookAngleAt(plan, 0.5f), 90.f);
}

TEST(LookAroundTest, AngleNeverLeavesTheSweep)
{
	LookPlan plan = Plan(0.f, 40.f, 3.f);

	for (float elapsed = 0.f; elapsed <= 3.f; elapsed += 0.05f)
	{
		float angle = LookAngleAt(plan, elapsed);

		EXPECT_LE(angle, 40.f + 0.01f) << elapsed;
		EXPECT_GE(angle, -40.f - 0.01f) << elapsed;
	}
}

TEST(LookAroundTest, TimeBeforeTheStartIsTheBaseAngle)
{
	EXPECT_FLOAT_EQ(LookAngleAt(Plan(), -1.f), 90.f);
}
