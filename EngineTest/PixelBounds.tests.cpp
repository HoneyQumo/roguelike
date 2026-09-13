#include "pch.h"
#include "PixelBounds.h"
#include <vector>

using XYZEngine::OpaqueBounds;

namespace
{
	class Canvas
	{
	public:
		Canvas(int width, int height) : width(width), height(height), pixels(static_cast<std::size_t>(width) * height * 4, 0)
		{
		}

		void Fill(const sf::IntRect& area, sf::Uint8 alpha)
		{
			for (int y = area.top; y < area.top + area.height; ++y)
			{
				for (int x = area.left; x < area.left + area.width; ++x)
				{
					pixels[(static_cast<std::size_t>(y) * width + x) * 4 + 3] = alpha;
				}
			}
		}

		sf::IntRect Bounds(const sf::IntRect& area, sf::Uint8 threshold = XYZEngine::DEFAULT_ALPHA_THRESHOLD) const
		{
			return OpaqueBounds(pixels.data(), width, height, area, threshold);
		}

	private:
		int width;
		int height;
		std::vector<sf::Uint8> pixels;
	};

	void ExpectRect(const sf::IntRect& actual, const sf::IntRect& expected)
	{
		EXPECT_EQ(actual.left, expected.left);
		EXPECT_EQ(actual.top, expected.top);
		EXPECT_EQ(actual.width, expected.width);
		EXPECT_EQ(actual.height, expected.height);
	}
}

TEST(PixelBoundsTest, TransparentPaddingIsCutAway)
{
	Canvas canvas(64, 32);
	canvas.Fill({20, 8, 10, 6}, 255);

	ExpectRect(canvas.Bounds({0, 0, 64, 32}), {20, 8, 10, 6});
}

TEST(PixelBoundsTest, SearchStaysInsideTheArea)
{
	Canvas canvas(64, 32);
	canvas.Fill({2, 2, 4, 4}, 255);
	canvas.Fill({40, 10, 8, 8}, 255);

	ExpectRect(canvas.Bounds({32, 0, 32, 32}), {40, 10, 8, 8});
}

TEST(PixelBoundsTest, FullyOpaqueAreaKeepsItsSize)
{
	Canvas canvas(16, 16);
	canvas.Fill({0, 0, 16, 16}, 255);

	ExpectRect(canvas.Bounds({0, 0, 16, 16}), {0, 0, 16, 16});
}

TEST(PixelBoundsTest, EmptyAreaFallsBackToTheAskedRect)
{
	Canvas canvas(16, 16);

	ExpectRect(canvas.Bounds({4, 4, 8, 8}), {4, 4, 8, 8});
}

TEST(PixelBoundsTest, AlmostTransparentPixelsAreNotContent)
{
	Canvas canvas(16, 16);
	canvas.Fill({0, 0, 16, 16}, 4);
	canvas.Fill({6, 6, 2, 2}, 200);

	ExpectRect(canvas.Bounds({0, 0, 16, 16}), {6, 6, 2, 2});
}

TEST(PixelBoundsTest, AreaOutsideTheImageFallsBackToTheAskedRect)
{
	Canvas canvas(16, 16);
	canvas.Fill({0, 0, 16, 16}, 255);

	ExpectRect(canvas.Bounds({40, 40, 8, 8}), {40, 40, 8, 8});
}

TEST(PixelBoundsTest, AreaIsClampedToTheImage)
{
	Canvas canvas(16, 16);
	canvas.Fill({12, 12, 4, 4}, 255);

	ExpectRect(canvas.Bounds({8, 8, 32, 32}), {12, 12, 4, 4});
}

TEST(PixelBoundsTest, MissingPixelsFallBackToTheAskedRect)
{
	ExpectRect(OpaqueBounds(nullptr, 16, 16, {0, 0, 16, 16}), {0, 0, 16, 16});
}
