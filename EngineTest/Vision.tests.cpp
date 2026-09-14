#include "pch.h"
#include "Vision.h"

using RoguelikeGame::CanSeeTarget;
using RoguelikeGame::IsWithinCone;
using RoguelikeGame::VisionCone;
using XYZEngine::Vector2Df;

namespace
{
	VisionCone Cone(float distance = 300.f, float halfAngle = 90.f)
	{
		VisionCone cone;
		cone.maxDistance = distance;
		cone.halfAngleDegrees = halfAngle;

		return cone;
	}

	const Vector2Df RIGHT = {1.f, 0.f};
}

TEST(VisionTest, TargetStraightAheadIsSeen)
{
	EXPECT_TRUE(CanSeeTarget(Cone(), RIGHT, {200.f, 0.f}, false));
}

TEST(VisionTest, TargetBehindIsNotSeen)
{
	EXPECT_FALSE(CanSeeTarget(Cone(), RIGHT, {-200.f, 0.f}, false));
}

TEST(VisionTest, TargetBeyondTheDistanceIsNotSeen)
{
	EXPECT_FALSE(CanSeeTarget(Cone(300.f), RIGHT, {400.f, 0.f}, false));
}

TEST(VisionTest, WallHidesTheTarget)
{
	EXPECT_FALSE(CanSeeTarget(Cone(), RIGHT, {200.f, 0.f}, true));
}

TEST(VisionTest, ConeEdgeIsInclusive)
{
	EXPECT_TRUE(IsWithinCone(RIGHT, {100.f, 99.f}, 45.f));
	EXPECT_FALSE(IsWithinCone(RIGHT, {99.f, 100.f}, 45.f));
}

TEST(VisionTest, NarrowConeCutsTheSides)
{
	EXPECT_TRUE(CanSeeTarget(Cone(300.f, 40.f), RIGHT, {200.f, 50.f}, false));
	EXPECT_FALSE(CanSeeTarget(Cone(300.f, 40.f), RIGHT, {200.f, 250.f}, false));
}

TEST(VisionTest, FullCircleSeesEverythingInRange)
{
	EXPECT_TRUE(CanSeeTarget(Cone(300.f, 180.f), RIGHT, {-200.f, 0.f}, false));
	EXPECT_FALSE(CanSeeTarget(Cone(300.f, 180.f), RIGHT, {-400.f, 0.f}, false));
}

TEST(VisionTest, BlindEnemySeesNothing)
{
	EXPECT_FALSE(CanSeeTarget(Cone(0.f, 180.f), RIGHT, {10.f, 0.f}, false));
	EXPECT_FALSE(IsWithinCone(RIGHT, {100.f, 0.f}, 0.f));
}

TEST(VisionTest, TargetOnTopOfTheEnemyIsSeen)
{
	EXPECT_TRUE(CanSeeTarget(Cone(300.f, 40.f), RIGHT, {0.f, 0.f}, false));
}
