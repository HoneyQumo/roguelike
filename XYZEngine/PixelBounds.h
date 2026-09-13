#pragma once

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace XYZEngine
{
	constexpr sf::Uint8 DEFAULT_ALPHA_THRESHOLD = 8;

	sf::IntRect OpaqueBounds(const sf::Uint8* pixels, int imageWidth, int imageHeight, const sf::IntRect& area,
		sf::Uint8 alphaThreshold = DEFAULT_ALPHA_THRESHOLD);

	sf::IntRect OpaqueBounds(const sf::Image& image, const sf::IntRect& area,
		sf::Uint8 alphaThreshold = DEFAULT_ALPHA_THRESHOLD);
}
