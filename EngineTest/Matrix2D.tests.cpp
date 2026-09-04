#include "pch.h"
#include "Matrix2D.h"
#include "MathUtils.h"

using XYZEngine::Matrix2D;
using XYZEngine::Vector2Df;

namespace
{
	constexpr float TOLERANCE = 1e-4f;

	void ExpectIdentity(const Matrix2D& matrix)
	{
		const auto& m = matrix.GetMatrix();
		for (int row = 0; row < 3; row++)
		{
			for (int column = 0; column < 3; column++)
			{
				float expected = row == column ? 1.f : 0.f;
				EXPECT_NEAR(m[row][column], expected, TOLERANCE) << "row " << row << ", column " << column;
			}
		}
	}
}

TEST(Matrix2DTests, DefaultIsIdentity)
{
	ExpectIdentity(Matrix2D());
}

TEST(Matrix2DTests, IdentityIsNeutralForMultiplication)
{
	Matrix2D transform({ 12.f, -5.f }, 30.f, { 2.f, 3.f });
	Matrix2D result = transform * Matrix2D();

	const auto& expected = transform.GetMatrix();
	const auto& actual = result.GetMatrix();
	for (int row = 0; row < 3; row++)
	{
		for (int column = 0; column < 3; column++)
		{
			EXPECT_NEAR(actual[row][column], expected[row][column], TOLERANCE);
		}
	}
}

TEST(Matrix2DTests, MultiplicationIsAssociative)
{
	Matrix2D first({ 3.f, 4.f }, 15.f, { 1.5f, 0.5f });
	Matrix2D second({ -7.f, 2.f }, -40.f, { 2.f, 2.f });
	Matrix2D third({ 1.f, 1.f }, 90.f, { 0.5f, 4.f });

	const auto& left = ((first * second) * third).GetMatrix();
	const auto& right = (first * (second * third)).GetMatrix();

	for (int row = 0; row < 3; row++)
	{
		for (int column = 0; column < 3; column++)
		{
			EXPECT_NEAR(left[row][column], right[row][column], TOLERANCE);
		}
	}
}

TEST(Matrix2DTests, InversedUndoesTransform)
{
	Matrix2D transform({ 25.f, -11.f }, 37.f, { 2.f, 0.5f });
	ExpectIdentity(transform * transform.GetInversed());
}

TEST(Matrix2DTests, InversedOfIdentityIsIdentity)
{
	ExpectIdentity(Matrix2D().GetInversed());
}

TEST(Matrix2DTests, InversedOfDegenerateMatrixIsFinite)
{
	Matrix2D degenerate({ 10.f, 10.f }, 0.f, { 0.f, 0.f });

	const auto& m = degenerate.GetInversed().GetMatrix();
	for (int row = 0; row < 3; row++)
	{
		for (int column = 0; column < 3; column++)
		{
			EXPECT_TRUE(std::isfinite(m[row][column])) << "row " << row << ", column " << column;
		}
	}
}

TEST(Matrix2DTests, InversedUndoesTranslationAndRotationInBothOrders)
{
	Matrix2D transform({ -140.f, 96.f }, -123.f, { 1.5f, 3.f });

	ExpectIdentity(transform.GetInversed() * transform);
	ExpectIdentity(transform * transform.GetInversed());
}

TEST(Matrix2DTests, InversedKeepsTheAffineBottomRow)
{
	Matrix2D transform({ 12.f, -7.f }, 44.f, { 0.75f, 2.f });

	const auto& m = transform.GetInversed().GetMatrix();
	EXPECT_FLOAT_EQ(m[2][0], 0.f);
	EXPECT_FLOAT_EQ(m[2][1], 0.f);
	EXPECT_FLOAT_EQ(m[2][2], 1.f);
}

TEST(MathUtilsTests, DegreesAndRadiansRoundTrip)
{
	for (float degrees : { 0.f, 30.f, 90.f, 180.f, -45.f, 359.f })
	{
		EXPECT_NEAR(XYZEngine::ToDegrees(XYZEngine::ToRadians(degrees)), degrees, TOLERANCE);
	}
}

TEST(MathUtilsTests, DirectionAndDegreesRoundTrip)
{
	for (float degrees : { 0.f, 45.f, 90.f, 179.f, -90.f })
	{
		Vector2Df direction = XYZEngine::DirectionFromDegrees(degrees);
		EXPECT_NEAR(XYZEngine::DegreesFromDirection(direction), degrees, 1e-3f);
	}
}

TEST(MathUtilsTests, RotateByDegreesKeepsLength)
{
	Vector2Df direction = { 3.f, 4.f };
	Vector2Df rotated = XYZEngine::RotateByDegrees(direction, 57.f);

	EXPECT_NEAR(rotated.GetLength(), direction.GetLength(), TOLERANCE);
}
