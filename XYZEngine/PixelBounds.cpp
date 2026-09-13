#include "pch.h"
#include "PixelBounds.h"

#include <algorithm>

namespace XYZEngine
{
	namespace
	{
		sf::IntRect ClampToImage(const sf::IntRect& area, int imageWidth, int imageHeight)
		{
			int left = std::max(area.left, 0);
			int top = std::max(area.top, 0);
			int right = std::min(area.left + area.width, imageWidth);
			int bottom = std::min(area.top + area.height, imageHeight);

			return {left, top, std::max(right - left, 0), std::max(bottom - top, 0)};
		}
	}

	sf::IntRect OpaqueBounds(const sf::Uint8* pixels, int imageWidth, int imageHeight, const sf::IntRect& area,
		sf::Uint8 alphaThreshold)
	{
		sf::IntRect scan = ClampToImage(area, imageWidth, imageHeight);

		if (pixels == nullptr || scan.width <= 0 || scan.height <= 0)
		{
			return area;
		}

		int left = scan.left + scan.width;
		int top = scan.top + scan.height;
		int right = scan.left - 1;
		int bottom = scan.top - 1;

		for (int y = scan.top; y < scan.top + scan.height; ++y)
		{
			for (int x = scan.left; x < scan.left + scan.width; ++x)
			{
				if (pixels[(static_cast<std::size_t>(y) * imageWidth + x) * 4 + 3] <= alphaThreshold)
				{
					continue;
				}

				left = std::min(left, x);
				top = std::min(top, y);
				right = std::max(right, x);
				bottom = std::max(bottom, y);
			}
		}

		if (right < left || bottom < top)
		{
			return area;
		}

		return {left, top, right - left + 1, bottom - top + 1};
	}

	sf::IntRect OpaqueBounds(const sf::Image& image, const sf::IntRect& area, sf::Uint8 alphaThreshold)
	{
		sf::Vector2u size = image.getSize();

		return OpaqueBounds(image.getPixelsPtr(), static_cast<int>(size.x), static_cast<int>(size.y), area, alphaThreshold);
	}
}
