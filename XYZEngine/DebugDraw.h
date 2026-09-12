#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Vector.h"

namespace XYZEngine
{
	class DebugDraw
	{
	public:
		static DebugDraw* Instance();

		void SetEnabled(bool newIsEnabled);
		bool IsEnabled() const;
		void Toggle();

		void SetFont(const sf::Font* newFont);

		void DrawCircle(const Vector2Df& center, float radius, const sf::Color& color);
		void DrawRect(const sf::FloatRect& rect, const sf::Color& color);
		void DrawLine(const Vector2Df& from, const Vector2Df& to, const sf::Color& color);
		void AddText(const std::string& line);

		std::size_t GetQueuedShapesCount() const;

		void Render();

	private:
		struct Circle
		{
			Vector2Df center;
			float radius;
			sf::Color color;
		};
		struct Rect
		{
			sf::FloatRect rect;
			sf::Color color;
		};
		struct Line
		{
			Vector2Df from;
			Vector2Df to;
			sf::Color color;
		};

		bool isEnabled = false;
		const sf::Font* font = nullptr;
		float smoothedFrameTime = 0.f;

		std::vector<Circle> circles;
		std::vector<Rect> rects;
		std::vector<Line> lines;
		std::vector<std::string> textLines;

		DebugDraw() {}
		~DebugDraw() {}

		DebugDraw(DebugDraw const&) = delete;
		DebugDraw& operator=(DebugDraw const&) = delete;

		void RenderColliders();
		void RenderShapes();
		void RenderText();
		void Clear();
	};
}
