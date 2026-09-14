#include "pch.h"
#include "PatrolRules.h"

using RoguelikeGame::PatrolStop;

using RoguelikeGame::HasReachedPatrolPoint;
using RoguelikeGame::NearestPatrolIndex;
using RoguelikeGame::NextPatrolIndex;
using XYZEngine::Vector2Df;

namespace
{
	const std::vector<PatrolStop> SQUARE = {{{0.f, 0.f}, false}, {{100.f, 0.f}, true}, {{100.f, 100.f}, false}, {{0.f, 100.f}, true}};
}

TEST(PatrolRulesTest, RouteLoopsBackToTheStart)
{
	EXPECT_EQ(NextPatrolIndex(0u, 4u), 1u);
	EXPECT_EQ(NextPatrolIndex(2u, 4u), 3u);
	EXPECT_EQ(NextPatrolIndex(3u, 4u), 0u);
}

TEST(PatrolRulesTest, EmptyRouteHasNoNextPoint)
{
	EXPECT_EQ(NextPatrolIndex(0u, 0u), 0u);
}

TEST(PatrolRulesTest, SinglePointRouteStaysOnIt)
{
	EXPECT_EQ(NextPatrolIndex(0u, 1u), 0u);
}

TEST(PatrolRulesTest, NearestPointIsPickedForTheReturn)
{
	EXPECT_EQ(NearestPatrolIndex(SQUARE, {90.f, 95.f}), 2u);
	EXPECT_EQ(NearestPatrolIndex(SQUARE, {-50.f, -50.f}), 0u);
	EXPECT_EQ(NearestPatrolIndex(SQUARE, {10.f, 90.f}), 3u);
}

TEST(PatrolRulesTest, NearestOfNothingIsTheStart)
{
	EXPECT_EQ(NearestPatrolIndex({}, {10.f, 10.f}), 0u);
}

TEST(PatrolRulesTest, ArrivalIsMeasuredByDistance)
{
	EXPECT_TRUE(HasReachedPatrolPoint({100.f, 0.f}, {90.f, 0.f}, 32.f));
	EXPECT_FALSE(HasReachedPatrolPoint({100.f, 0.f}, {50.f, 0.f}, 32.f));
}

TEST(PatrolRulesTest, ArrivalEdgeCounts)
{
	EXPECT_TRUE(HasReachedPatrolPoint({32.f, 0.f}, {0.f, 0.f}, 32.f));
}
