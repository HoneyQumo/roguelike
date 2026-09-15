#include "pch.h"
#include "AwarenessGauge.h"
#include <cmath>

using RoguelikeGame::AwarenessState;
using RoguelikeGame::GAUGE_FLASH_TIME;
using RoguelikeGame::GAUGE_STEPS;
using RoguelikeGame::GaugeLook;
using RoguelikeGame::LookFor;
using RoguelikeGame::SectorPoints;
using XYZEngine::Vector2Df;

namespace
{
	const Vector2Df CENTER = {100.f, 200.f};

	float DistanceFromCenter(const Vector2Df& point)
	{
		return std::sqrt((point.x - CENTER.x) * (point.x - CENTER.x) + (point.y - CENTER.y) * (point.y - CENTER.y));
	}
}

TEST(AwarenessGaugeTest, CalmEnemyShowsNothing)
{
	GaugeLook look = LookFor(0.f, AwarenessState::Calm, 0.f);

	EXPECT_FALSE(look.isShown);
	EXPECT_FLOAT_EQ(look.part, 0.f);
}

TEST(AwarenessGaugeTest, SuspiciousEnemyShowsAPartOfTheCircle)
{
	GaugeLook look = LookFor(RoguelikeGame::AWARENESS_ALERT_AT, AwarenessState::Alerted, 0.f);

	EXPECT_TRUE(look.isShown);
	EXPECT_FALSE(look.isAlarm);
	EXPECT_GT(look.part, 0.f);
	EXPECT_LT(look.part, 1.f);
}

TEST(AwarenessGaugeTest, SpottingFlashesTheWholeCircle)
{
	GaugeLook look = LookFor(RoguelikeGame::AWARENESS_PROVOKE_AT, AwarenessState::Provoked, 0.f);

	EXPECT_TRUE(look.isShown);
	EXPECT_TRUE(look.isAlarm);
	EXPECT_FLOAT_EQ(look.part, 1.f);
	EXPECT_FLOAT_EQ(look.fade, 1.f);
}

TEST(AwarenessGaugeTest, FlashFadesOutAndGoesAway)
{
	GaugeLook middle = LookFor(2.f, AwarenessState::Provoked, GAUGE_FLASH_TIME * 0.5f);
	GaugeLook over = LookFor(2.f, AwarenessState::Provoked, GAUGE_FLASH_TIME * 2.f);

	EXPECT_TRUE(middle.isShown);
	EXPECT_LT(middle.fade, 1.f);
	EXPECT_GT(middle.fade, 0.f);
	EXPECT_FALSE(over.isShown);
}

TEST(AwarenessGaugeTest, EmptyScaleDrawsNoSector)
{
	EXPECT_TRUE(SectorPoints(CENTER, 10.f, 0.f, GAUGE_STEPS).empty());
	EXPECT_TRUE(SectorPoints(CENTER, 0.f, 1.f, GAUGE_STEPS).empty());
	EXPECT_TRUE(SectorPoints(CENTER, 10.f, 1.f, 0).empty());
}

TEST(AwarenessGaugeTest, SectorStartsAtTheCenter)
{
	std::vector<Vector2Df> points = SectorPoints(CENTER, 10.f, 0.5f, GAUGE_STEPS);

	ASSERT_FALSE(points.empty());
	EXPECT_FLOAT_EQ(points[0].x, CENTER.x);
	EXPECT_FLOAT_EQ(points[0].y, CENTER.y);
}

TEST(AwarenessGaugeTest, EveryEdgePointSitsOnTheCircle)
{
	std::vector<Vector2Df> points = SectorPoints(CENTER, 10.f, 0.75f, GAUGE_STEPS);

	ASSERT_GT(points.size(), 2u);
	for (std::size_t at = 1u; at < points.size(); at++)
	{
		EXPECT_NEAR(DistanceFromCenter(points[at]), 10.f, 0.01f);
	}
}

TEST(AwarenessGaugeTest, SectorStartsAtTheTop)
{
	std::vector<Vector2Df> points = SectorPoints(CENTER, 10.f, 0.25f, GAUGE_STEPS);

	ASSERT_GT(points.size(), 1u);
	EXPECT_NEAR(points[1].x, CENTER.x, 0.01f);
	EXPECT_NEAR(points[1].y, CENTER.y + 10.f, 0.01f);
}

TEST(AwarenessGaugeTest, QuarterTurnsToTheRightSide)
{
	std::vector<Vector2Df> points = SectorPoints(CENTER, 10.f, 0.25f, GAUGE_STEPS);

	ASSERT_GT(points.size(), 2u);
	EXPECT_NEAR(points.back().x, CENTER.x + 10.f, 0.01f);
	EXPECT_NEAR(points.back().y, CENTER.y, 0.01f);
}

TEST(AwarenessGaugeTest, FullerScaleDrawsMorePoints)
{
	EXPECT_LT(SectorPoints(CENTER, 10.f, 0.25f, GAUGE_STEPS).size(), SectorPoints(CENTER, 10.f, 1.f, GAUGE_STEPS).size());
}

TEST(AwarenessGaugeTest, WholeCircleComesBackToTheTop)
{
	std::vector<Vector2Df> points = SectorPoints(CENTER, 10.f, 1.f, GAUGE_STEPS);

	ASSERT_GT(points.size(), 2u);
	EXPECT_NEAR(points.back().x, CENTER.x, 0.01f);
	EXPECT_NEAR(points.back().y, CENTER.y + 10.f, 0.01f);
}
