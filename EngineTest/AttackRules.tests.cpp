#include "pch.h"
#include "AttackRules.h"

using RoguelikeGame::AttackSense;
using RoguelikeGame::MayAttack;

namespace
{
	AttackSense Ready()
	{
		AttackSense sense;
		sense.isAlive = true;
		sense.hasTarget = true;
		sense.isTargetAlive = true;
		sense.canSeeTarget = true;
		sense.attackRange = 300.f;
		sense.distanceToTarget = 200.f;

		return sense;
	}
}

TEST(AttackRulesTest, SeenTargetInRangeIsAttacked)
{
	EXPECT_TRUE(MayAttack(Ready()));
}

TEST(AttackRulesTest, TargetBehindAWallIsNotAttacked)
{
	AttackSense sense = Ready();
	sense.canSeeTarget = false;

	EXPECT_FALSE(MayAttack(sense));
}

TEST(AttackRulesTest, TargetBeyondTheRangeIsNotAttacked)
{
	AttackSense sense = Ready();
	sense.distanceToTarget = 400.f;

	EXPECT_FALSE(MayAttack(sense));
}

TEST(AttackRulesTest, RangeEdgeStillCounts)
{
	AttackSense sense = Ready();
	sense.distanceToTarget = sense.attackRange;

	EXPECT_TRUE(MayAttack(sense));
}

TEST(AttackRulesTest, DeadEnemyDoesNotAttack)
{
	AttackSense sense = Ready();
	sense.isAlive = false;

	EXPECT_FALSE(MayAttack(sense));
}

TEST(AttackRulesTest, DeadTargetIsNotAttacked)
{
	AttackSense sense = Ready();
	sense.isTargetAlive = false;

	EXPECT_FALSE(MayAttack(sense));
}

TEST(AttackRulesTest, MissingTargetIsNotAttacked)
{
	AttackSense sense = Ready();
	sense.hasTarget = false;

	EXPECT_FALSE(MayAttack(sense));
}

TEST(AttackRulesTest, UnarmedEnemyDoesNotAttack)
{
	AttackSense sense = Ready();
	sense.attackRange = 0.f;

	EXPECT_FALSE(MayAttack(sense));
}
