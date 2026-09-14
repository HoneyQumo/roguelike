#include "pch.h"
#include "ChaseRules.h"

using RoguelikeGame::ChaseMove;
using RoguelikeGame::ChaseSense;
using RoguelikeGame::ChooseChaseMove;

namespace
{
	ChaseSense Enemy(float distanceToTarget, bool isVisible = false)
	{
		ChaseSense sense;
		sense.detectionRadius = 300.f;
		sense.stopDistance = 40.f;
		sense.arriveDistance = 48.f;
		sense.distanceToTarget = distanceToTarget;
		sense.isVisible = isVisible;

		return sense;
	}
}

TEST(ChaseRulesTest, TargetInsideTheRadiusIsChased)
{
	EXPECT_EQ(ChooseChaseMove(Enemy(200.f, true)), ChaseMove::Approach);
}

TEST(ChaseRulesTest, ChaseStopsAtTheStopDistance)
{
	EXPECT_EQ(ChooseChaseMove(Enemy(30.f, true)), ChaseMove::Hold);
}

TEST(ChaseRulesTest, UnseenTargetIsIgnored)
{
	EXPECT_EQ(ChooseChaseMove(Enemy(900.f)), ChaseMove::Hold);
	EXPECT_EQ(ChooseChaseMove(Enemy(100.f)), ChaseMove::Hold);
}

TEST(ChaseRulesTest, AlertedEnemyWalksToThePointWhileTheTargetIsUnseen)
{
	ChaseSense sense = Enemy(900.f);
	sense.isAlerted = true;
	sense.hasPoint = true;
	sense.distanceToPoint = 500.f;

	EXPECT_EQ(ChooseChaseMove(sense), ChaseMove::Investigate);
}

TEST(ChaseRulesTest, ArrivingAtThePointEndsTheWalk)
{
	ChaseSense sense = Enemy(900.f);
	sense.isAlerted = true;
	sense.hasPoint = true;
	sense.distanceToPoint = 10.f;

	EXPECT_EQ(ChooseChaseMove(sense), ChaseMove::Hold);
}

TEST(ChaseRulesTest, SeeingTheTargetWinsOverThePoint)
{
	ChaseSense sense = Enemy(200.f, true);
	sense.isAlerted = true;
	sense.hasPoint = true;
	sense.distanceToPoint = 500.f;

	EXPECT_EQ(ChooseChaseMove(sense), ChaseMove::Approach);
}

TEST(ChaseRulesTest, AlertWithoutAPointKeepsTheEnemyInPlace)
{
	ChaseSense sense = Enemy(900.f);
	sense.isAlerted = true;

	EXPECT_EQ(ChooseChaseMove(sense), ChaseMove::Hold);
}

TEST(ChaseRulesTest, ForcedChaseIgnoresTheRadius)
{
	ChaseSense sense = Enemy(4000.f);
	sense.isForced = true;

	EXPECT_TRUE(RoguelikeGame::IsTargetDetected(sense));
	EXPECT_EQ(ChooseChaseMove(sense), ChaseMove::Approach);
}

TEST(ChaseRulesTest, EngagementCoversBothSightAndAlert)
{
	ChaseSense seen = Enemy(100.f, true);
	ChaseSense alerted = Enemy(900.f);
	alerted.isAlerted = true;
	ChaseSense quiet = Enemy(900.f);

	EXPECT_TRUE(RoguelikeGame::IsEngaged(seen));
	EXPECT_TRUE(RoguelikeGame::IsEngaged(alerted));
	EXPECT_FALSE(RoguelikeGame::IsEngaged(quiet));
}

TEST(ChaseRulesTest, MarauderCannotShootFromBeyondItsSight)
{
	ChaseSense marauder;
	marauder.detectionRadius = 160.f;
	marauder.stopDistance = 140.f;
	marauder.arriveDistance = 48.f;
	marauder.distanceToTarget = 200.f;

	EXPECT_FALSE(RoguelikeGame::IsEngaged(marauder));

	marauder.isAlerted = true;
	EXPECT_TRUE(RoguelikeGame::IsEngaged(marauder));
}
