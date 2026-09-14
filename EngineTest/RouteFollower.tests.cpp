#include "pch.h"
#include "RouteFollower.h"

using RoguelikeGame::RouteFollower;
using XYZEngine::Vector2Df;

namespace
{
	std::vector<Vector2Df> Line()
	{
		return {{0.f, 0.f}, {100.f, 0.f}, {200.f, 0.f}};
	}
}

TEST(RouteFollowerTest, FreshFollowerHasNoPoint)
{
	RouteFollower follower;

	EXPECT_FALSE(follower.HasPoint());
	EXPECT_EQ(follower.GetRemaining(), 0u);
}

TEST(RouteFollowerTest, FirstPointComesFirst)
{
	RouteFollower follower;
	follower.SetRoute(Line());

	ASSERT_TRUE(follower.HasPoint());
	EXPECT_EQ(follower.GetPoint().x, 0.f);
	EXPECT_EQ(follower.GetRemaining(), 3u);
}

TEST(RouteFollowerTest, ReachedPointIsDropped)
{
	RouteFollower follower;
	follower.SetRoute(Line());
	follower.Advance({5.f, 0.f}, 32.f);

	ASSERT_TRUE(follower.HasPoint());
	EXPECT_EQ(follower.GetPoint().x, 100.f);
}

TEST(RouteFollowerTest, FarPositionDropsNothing)
{
	RouteFollower follower;
	follower.SetRoute(Line());
	follower.Advance({-500.f, 0.f}, 32.f);

	EXPECT_EQ(follower.GetPoint().x, 0.f);
	EXPECT_EQ(follower.GetRemaining(), 3u);
}

TEST(RouteFollowerTest, SeveralPointsAtOnceAreSkipped)
{
	RouteFollower follower;
	follower.SetRoute(Line());
	follower.Advance({50.f, 0.f}, 60.f);

	ASSERT_TRUE(follower.HasPoint());
	EXPECT_EQ(follower.GetPoint().x, 200.f);
	EXPECT_EQ(follower.GetRemaining(), 1u);
}

TEST(RouteFollowerTest, WalkedRouteRunsOut)
{
	RouteFollower follower;
	follower.SetRoute(Line());
	follower.Advance({200.f, 0.f}, 500.f);

	EXPECT_FALSE(follower.HasPoint());
	EXPECT_EQ(follower.GetRemaining(), 0u);
}

TEST(RouteFollowerTest, NewRouteStartsFromItsFirstPoint)
{
	RouteFollower follower;
	follower.SetRoute(Line());
	follower.Advance({50.f, 0.f}, 60.f);
	follower.SetRoute(Line());

	EXPECT_EQ(follower.GetPoint().x, 0.f);
	EXPECT_EQ(follower.GetIndex(), 0u);
}

TEST(RouteFollowerTest, ClearedFollowerHasNoPoint)
{
	RouteFollower follower;
	follower.SetRoute(Line());
	follower.Clear();

	EXPECT_FALSE(follower.HasPoint());
}
