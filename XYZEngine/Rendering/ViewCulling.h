#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <algorithm>
#include <cmath>

namespace XYZEngine
{
	constexpr float VIEW_CULLING_MARGIN = 32.f;

	inline sf::FloatRect NormalizedArea(const sf::FloatRect& area)
	{
		return sf::FloatRect(std::min(area.left, area.left + area.width),
			std::min(area.top, area.top + area.height),
			std::abs(area.width), std::abs(area.height));
	}

	inline sf::FloatRect GrownBy(const sf::FloatRect& area, float margin)
	{
		return sf::FloatRect(area.left - margin, area.top - margin,
			area.width + 2.f * margin, area.height + 2.f * margin);
	}

	inline bool IsInView(const sf::FloatRect& bounds, const sf::FloatRect& view, float margin)
	{
		sf::FloatRect watched = NormalizedArea(view);
		if (watched.width <= 0.f || watched.height <= 0.f)
		{
			return true;
		}

		sf::FloatRect seen = GrownBy(watched, margin);
		sf::FloatRect shape = NormalizedArea(bounds);

		return shape.left <= seen.left + seen.width && seen.left <= shape.left + shape.width
			&& shape.top <= seen.top + seen.height && seen.top <= shape.top + shape.height;
	}
}
