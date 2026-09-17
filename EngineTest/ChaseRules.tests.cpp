#include "pch.h"
#include "ChaseRules.h"
#include "GameSettings.h"
#include <algorithm>

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

namespace
{
	// Штурмовик: встаёт на 220, пятится ближе 160.
	ChaseSense Shooter(float distanceToTarget)
	{
		ChaseSense sense;
		sense.detectionRadius = 420.f;
		sense.stopDistance = 220.f;
		sense.backOffDistance = 220.f - RoguelikeGame::ENEMY_COMFORT_DEAD_ZONE;
		sense.arriveDistance = 48.f;
		sense.distanceToTarget = distanceToTarget;
		sense.isVisible = true;

		return sense;
	}

	// Ножевик: stopDistance 40, мёртвая зона шире - отход недостижим.
	ChaseSense Knifeman(float distanceToTarget)
	{
		ChaseSense sense = Shooter(distanceToTarget);
		sense.stopDistance = 40.f;
		sense.backOffDistance = std::max(0.f, 40.f - RoguelikeGame::ENEMY_COMFORT_DEAD_ZONE);

		return sense;
	}
}

TEST(ChaseRulesTest, AShooterBacksOffWhenTheTargetGetsTooClose)
{
	EXPECT_EQ(ChooseChaseMove(Shooter(100.f)), ChaseMove::Withdraw);
}

TEST(ChaseRulesTest, AShooterHoldsInsideTheDeadZone)
{
	EXPECT_EQ(ChooseChaseMove(Shooter(200.f)), ChaseMove::Hold);
	EXPECT_EQ(ChooseChaseMove(Shooter(165.f)), ChaseMove::Hold);
}

TEST(ChaseRulesTest, AShooterStillWalksUpFromAfar)
{
	EXPECT_EQ(ChooseChaseMove(Shooter(300.f)), ChaseMove::Approach);
}

TEST(ChaseRulesTest, AKnifemanNeverBacksOff)
{
	for (float distance : {1.f, 10.f, 25.f, 39.f})
	{
		EXPECT_EQ(ChooseChaseMove(Knifeman(distance)), ChaseMove::Hold) << "distance " << distance;
	}
}

TEST(ChaseRulesTest, ABackingShooterKeepsTheTargetInSight)
{
	ChaseSense sense = Shooter(100.f);

	EXPECT_EQ(ChooseChaseMove(sense), ChaseMove::Withdraw);
	EXPECT_TRUE(RoguelikeGame::IsTargetDetected(sense)) << "backing off is not the same as losing the target";
}

TEST(ChaseRulesTest, AnUnseenTargetIsNotBackedAwayFrom)
{
	ChaseSense sense = Shooter(100.f);
	sense.isVisible = false;
	sense.isAlerted = true;

	EXPECT_NE(ChooseChaseMove(sense), ChaseMove::Withdraw) << "the enemy retreats from a target it cannot see";
}

namespace
{
	ChaseSense Reloading(bool hasCover)
	{
		ChaseSense sense = Shooter(300.f);
		sense.isReloading = true;
		sense.hasCover = hasCover;

		return sense;
	}
}

TEST(ChaseRulesTest, AReloadingShooterGoesForCover)
{
	EXPECT_EQ(ChooseChaseMove(Reloading(true)), ChaseMove::TakeCover);
}

TEST(ChaseRulesTest, CoverBeatsEverythingElseWhileTheMagazineIsEmpty)
{
	ChaseSense close = Reloading(true);
	close.distanceToTarget = 100.f;

	EXPECT_EQ(ChooseChaseMove(close), ChaseMove::TakeCover) << "the enemy backed off instead of hiding";
}

TEST(ChaseRulesTest, AHiddenReloaderDoesNotRunOffToSearch)
{
	ChaseSense sense = Reloading(true);
	sense.isVisible = false;
	sense.isAlerted = true;
	sense.hasPoint = true;
	sense.distanceToPoint = 400.f;

	EXPECT_EQ(ChooseChaseMove(sense), ChaseMove::TakeCover) << "the enemy left cover to look for a target it just lost";
}

TEST(ChaseRulesTest, WithoutCoverTheReloaderFightsOnAsBefore)
{
	EXPECT_EQ(ChooseChaseMove(Reloading(false)), ChaseMove::Approach);
}

TEST(ChaseRulesTest, ALoadedShooterStaysOutOfCover)
{
	ChaseSense sense = Shooter(300.f);
	sense.hasCover = true;

	EXPECT_EQ(ChooseChaseMove(sense), ChaseMove::Approach);
}
