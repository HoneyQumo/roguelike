#include "pch.h"
#include "Trail.h"
#include <cmath>

using RoguelikeGame::BuildTrail;
using RoguelikeGame::TrailPoint;
using RoguelikeGame::TrailQuad;
using RoguelikeGame::TrailSide;
using XYZEngine::Vector2Df;

namespace
{
	constexpr float WIDTH = 18.f;

	std::vector<TrailPoint> Straight(int count)
	{
		std::vector<TrailPoint> points;
		for (int index = 0; index < count; index++)
		{
			points.push_back({{index * 32.f, 0.f}, WIDTH, 1.f});
		}

		return points;
	}

	float Gap(const Vector2Df& first, const Vector2Df& second)
	{
		return (first - second).GetLength();
	}
}

TEST(TrailTest, ASinglePointIsNotARibbon)
{
	EXPECT_TRUE(BuildTrail(Straight(1)).empty());
	EXPECT_TRUE(BuildTrail({}).empty());
}

TEST(TrailTest, EveryPairOfPointsGivesOneQuad)
{
	EXPECT_EQ(BuildTrail(Straight(2)).size(), 1u);
	EXPECT_EQ(BuildTrail(Straight(5)).size(), 4u);
}

TEST(TrailTest, TheRibbonIsAsWideAsItWasAsked)
{
	std::vector<TrailQuad> quads = BuildTrail(Straight(2));

	ASSERT_EQ(quads.size(), 1u);
	EXPECT_NEAR(Gap(quads[0].corners[0], quads[0].corners[3]), WIDTH, 0.01f);
	EXPECT_NEAR(Gap(quads[0].corners[1], quads[0].corners[2]), WIDTH, 0.01f);
}

/**
*	Главное свойство ленты: соседние квады делят ровно одни и те же углы.
*
*	Отдельные штампы этого не дают - между ними либо щель, либо наложение,
*	и след читается пунктиром.
*/
TEST(TrailTest, NeighbouringQuadsShareTheirCorners)
{
	std::vector<TrailPoint> points = {
		{{0.f, 0.f}, WIDTH, 1.f},
		{{40.f, 10.f}, WIDTH, 1.f},
		{{80.f, 40.f}, WIDTH, 1.f},
		{{100.f, 90.f}, WIDTH, 1.f},
	};

	std::vector<TrailQuad> quads = BuildTrail(points);
	ASSERT_EQ(quads.size(), 3u);

	for (std::size_t index = 0u; index + 1u < quads.size(); index++)
	{
		EXPECT_NEAR(Gap(quads[index].corners[1], quads[index + 1u].corners[0]), 0.f, 0.001f)
			<< "a gap on the right side between quads " << index << " and " << index + 1u;
		EXPECT_NEAR(Gap(quads[index].corners[2], quads[index + 1u].corners[3]), 0.f, 0.001f)
			<< "a gap on the left side between quads " << index << " and " << index + 1u;
	}
}

TEST(TrailTest, AStraightRibbonKeepsItsWidthOnEveryJoint)
{
	std::vector<TrailQuad> quads = BuildTrail(Straight(4));

	for (const TrailQuad& quad : quads)
	{
		EXPECT_NEAR(Gap(quad.corners[0], quad.corners[3]), WIDTH, 0.01f);
		EXPECT_NEAR(Gap(quad.corners[1], quad.corners[2]), WIDTH, 0.01f);
	}
}

TEST(TrailTest, ASharpTurnDoesNotStretchTheJointToInfinity)
{
	// Разворот почти на месте: ус стыка без ограничения ушёл бы в бесконечность.
	std::vector<TrailPoint> points = {
		{{0.f, 0.f}, WIDTH, 1.f},
		{{40.f, 0.f}, WIDTH, 1.f},
		{{0.1f, 1.f}, WIDTH, 1.f},
	};

	std::vector<TrailQuad> quads = BuildTrail(points);
	ASSERT_EQ(quads.size(), 2u);

	for (const TrailQuad& quad : quads)
	{
		for (const Vector2Df& corner : quad.corners)
		{
			EXPECT_TRUE(std::isfinite(corner.x) && std::isfinite(corner.y));
			EXPECT_LT(Gap(corner, {20.f, 0.f}), 10.f * WIDTH) << "the joint flew off the map";
		}
	}
}

TEST(TrailTest, TheRibbonRunsAlongThePathAndNotAcross)
{
	std::vector<TrailQuad> quads = BuildTrail(Straight(2));

	ASSERT_EQ(quads.size(), 1u);

	// Путь идёт по x, значит края ленты разъезжаются по y.
	EXPECT_NEAR(quads[0].corners[0].x, 0.f, 0.01f);
	EXPECT_NEAR(quads[0].corners[3].x, 0.f, 0.01f);
	EXPECT_NEAR(std::abs(quads[0].corners[0].y - quads[0].corners[3].y), WIDTH, 0.01f);
}

TEST(TrailTest, TheDarknessOfAJointIsTakenFromItsEnds)
{
	std::vector<TrailPoint> points = {
		{{0.f, 0.f}, WIDTH, 0.4f},
		{{32.f, 0.f}, WIDTH, 1.f},
	};

	std::vector<TrailQuad> quads = BuildTrail(points);

	ASSERT_EQ(quads.size(), 1u);
	EXPECT_NEAR(quads[0].alpha, 0.7f, 0.001f);
}

TEST(TrailTest, APointOnTopOfAnotherDoesNotBreakTheRibbon)
{
	std::vector<TrailPoint> points = {
		{{0.f, 0.f}, WIDTH, 1.f},
		{{0.f, 0.f}, WIDTH, 1.f},
		{{32.f, 0.f}, WIDTH, 1.f},
	};

	std::vector<TrailQuad> quads = BuildTrail(points);

	ASSERT_EQ(quads.size(), 2u);
	for (const TrailQuad& quad : quads)
	{
		for (const Vector2Df& corner : quad.corners)
		{
			EXPECT_TRUE(std::isfinite(corner.x) && std::isfinite(corner.y));
		}
	}
}

/**
*	На повороте стык обязан растянуться, иначе лента там сузится.
*
*	Ширина мерится поперёк хода, а не по углам стыка: сам стык при этом
*	длиннее ширины ровно на 1/cos половины угла поворота.
*/
TEST(TrailTest, TheRibbonDoesNotPinchOnATurn)
{
	// Поворот на прямой угол: ус должен вырасти в корень из двух раз.
	std::vector<TrailPoint> points = {
		{{0.f, 0.f}, WIDTH, 1.f},
		{{40.f, 0.f}, WIDTH, 1.f},
		{{40.f, 40.f}, WIDTH, 1.f},
	};

	std::vector<TrailQuad> quads = BuildTrail(points);
	ASSERT_EQ(quads.size(), 2u);

	float joint = Gap(quads[0].corners[1], quads[0].corners[2]);

	EXPECT_NEAR(joint, WIDTH * std::sqrt(2.f), 0.5f)
		<< "the ribbon narrows on the turn instead of holding its width";
}
