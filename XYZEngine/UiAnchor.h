#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace XYZEngine
{
	enum class UiAnchor
	{
		TopLeft,
		Top,
		TopRight,
		Left,
		Center,
		Right,
		BottomLeft,
		Bottom,
		BottomRight
	};

	inline sf::Vector2f AnchorRatio(UiAnchor anchor)
	{
		switch (anchor)
		{
		case UiAnchor::TopLeft: return {0.f, 0.f};
		case UiAnchor::Top: return {0.5f, 0.f};
		case UiAnchor::TopRight: return {1.f, 0.f};
		case UiAnchor::Left: return {0.f, 0.5f};
		case UiAnchor::Center: return {0.5f, 0.5f};
		case UiAnchor::Right: return {1.f, 0.5f};
		case UiAnchor::BottomLeft: return {0.f, 1.f};
		case UiAnchor::Bottom: return {0.5f, 1.f};
		case UiAnchor::BottomRight: return {1.f, 1.f};
		}

		return {0.f, 0.f};
	}

	inline sf::Vector2f AnchorPoint(const sf::FloatRect& rect, UiAnchor anchor)
	{
		sf::Vector2f ratio = AnchorRatio(anchor);
		return {rect.left + rect.width * ratio.x, rect.top + rect.height * ratio.y};
	}
}
