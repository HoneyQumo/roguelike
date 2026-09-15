#include "pch.h"
#include "ViewCulling.h"

using XYZEngine::GrownBy;
using XYZEngine::IsInView;
using XYZEngine::NormalizedArea;

namespace
{
	const sf::FloatRect VIEW = sf::FloatRect(0.f, 0.f, 800.f, 600.f);

	sf::FloatRect Box(float left, float top, float size)
	{
		return sf::FloatRect(left, top, size, size);
	}
}

TEST(ViewCullingTest, ShapeInTheMiddleIsSeen)
{
	EXPECT_TRUE(IsInView(Box(400.f, 300.f, 64.f), VIEW, 0.f));
}

TEST(ViewCullingTest, ShapeFarAwayIsNotSeen)
{
	EXPECT_FALSE(IsInView(Box(5000.f, 5000.f, 64.f), VIEW, 0.f));
	EXPECT_FALSE(IsInView(Box(-5000.f, 300.f, 64.f), VIEW, 0.f));
}

TEST(ViewCullingTest, ShapeOverTheEdgeIsSeenWhole)
{
	EXPECT_TRUE(IsInView(Box(-32.f, 300.f, 64.f), VIEW, 0.f));
	EXPECT_TRUE(IsInView(Box(780.f, 300.f, 64.f), VIEW, 0.f));
	EXPECT_TRUE(IsInView(Box(400.f, -20.f, 64.f), VIEW, 0.f));
}

TEST(ViewCullingTest, ShapeTouchingTheEdgeIsStillSeen)
{
	EXPECT_TRUE(IsInView(Box(-64.f, 300.f, 64.f), VIEW, 0.f));
	EXPECT_TRUE(IsInView(Box(800.f, 300.f, 64.f), VIEW, 0.f));
}

TEST(ViewCullingTest, MarginKeepsShapesJustOutside)
{
	EXPECT_FALSE(IsInView(Box(-160.f, 300.f, 64.f), VIEW, 0.f));
	EXPECT_TRUE(IsInView(Box(-160.f, 300.f, 64.f), VIEW, 100.f));
}

TEST(ViewCullingTest, FlippedViewIsTheSameArea)
{
	sf::FloatRect flipped(0.f, 600.f, 800.f, -600.f);

	EXPECT_TRUE(IsInView(Box(400.f, 300.f, 64.f), flipped, 0.f));
	EXPECT_FALSE(IsInView(Box(5000.f, 5000.f, 64.f), flipped, 0.f));
}

TEST(ViewCullingTest, EmptyViewHidesNothing)
{
	EXPECT_TRUE(IsInView(Box(5000.f, 5000.f, 64.f), sf::FloatRect(0.f, 0.f, 0.f, 0.f), 32.f));
}

TEST(ViewCullingTest, ShapeWithoutSizeIsSeenWhereItStands)
{
	EXPECT_TRUE(IsInView(sf::FloatRect(400.f, 300.f, 0.f, 0.f), VIEW, 0.f));
	EXPECT_FALSE(IsInView(sf::FloatRect(5000.f, 300.f, 0.f, 0.f), VIEW, 0.f));
}

TEST(ViewCullingTest, FlippedShapeIsSeenWhereItReallyIs)
{
	EXPECT_TRUE(IsInView(sf::FloatRect(50.f, 300.f, -100.f, -60.f), VIEW, 0.f));
	EXPECT_TRUE(IsInView(sf::FloatRect(400.f, 50.f, -60.f, -100.f), VIEW, 0.f));
	EXPECT_FALSE(IsInView(sf::FloatRect(-100.f, 300.f, -60.f, -60.f), VIEW, 0.f));
}

TEST(ViewCullingTest, NegativeSizeIsStraightenedOut)
{
	sf::FloatRect area = NormalizedArea(sf::FloatRect(100.f, 100.f, -40.f, -60.f));

	EXPECT_FLOAT_EQ(area.left, 60.f);
	EXPECT_FLOAT_EQ(area.top, 40.f);
	EXPECT_FLOAT_EQ(area.width, 40.f);
	EXPECT_FLOAT_EQ(area.height, 60.f);
}

TEST(ViewCullingTest, GrowingAddsMarginOnEverySide)
{
	sf::FloatRect area = GrownBy(sf::FloatRect(10.f, 20.f, 100.f, 200.f), 5.f);

	EXPECT_FLOAT_EQ(area.left, 5.f);
	EXPECT_FLOAT_EQ(area.top, 15.f);
	EXPECT_FLOAT_EQ(area.width, 110.f);
	EXPECT_FLOAT_EQ(area.height, 210.f);
}

