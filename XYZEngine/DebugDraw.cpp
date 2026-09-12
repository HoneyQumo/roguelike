#include "pch.h"
#include "DebugDraw.h"
#include "PhysicsSystem.h"
#include "GameWorld.h"
#include "RenderSystem.h"
#include "FrameClock.h"
#include "LoggerRegistry.h"
#include <cstdio>

namespace XYZEngine
{
	constexpr float DEBUG_OUTLINE_THICKNESS = 1.f;
	constexpr unsigned int DEBUG_TEXT_SIZE = 14;
	constexpr float DEBUG_TEXT_MARGIN = 8.f;
	constexpr float DEBUG_TEXT_LINE_HEIGHT = 18.f;
	constexpr float DEBUG_TEXT_OUTLINE = 1.f;
	constexpr float FRAME_TIME_SMOOTHING = 0.1f;
	const sf::Color DEBUG_COLLIDER_COLOR = {80, 220, 80};
	const sf::Color DEBUG_TRIGGER_COLOR = {80, 200, 255};
	const sf::Color DEBUG_TEXT_COLOR = {255, 255, 255};
	const sf::Color DEBUG_TEXT_OUTLINE_COLOR = {0, 0, 0, 200};

	DebugDraw* DebugDraw::Instance()
	{
		static DebugDraw debugDraw;
		return &debugDraw;
	}

	void DebugDraw::SetEnabled(bool newIsEnabled)
	{
		if (isEnabled == newIsEnabled)
		{
			return;
		}

		isEnabled = newIsEnabled;
		if (!isEnabled)
		{
			Clear();
		}

		LOG_INFO(std::string("Debug draw ") + (isEnabled ? "on" : "off"));
	}
	bool DebugDraw::IsEnabled() const
	{
		return isEnabled;
	}
	void DebugDraw::Toggle()
	{
		SetEnabled(!isEnabled);
	}

	void DebugDraw::SetFont(const sf::Font* newFont)
	{
		font = newFont;
	}

	void DebugDraw::DrawCircle(const Vector2Df& center, float radius, const sf::Color& color)
	{
		if (isEnabled)
		{
			circles.push_back({center, radius, color});
		}
	}
	void DebugDraw::DrawRect(const sf::FloatRect& rect, const sf::Color& color)
	{
		if (isEnabled)
		{
			rects.push_back({rect, color});
		}
	}
	void DebugDraw::DrawLine(const Vector2Df& from, const Vector2Df& to, const sf::Color& color)
	{
		if (isEnabled)
		{
			lines.push_back({from, to, color});
		}
	}
	void DebugDraw::AddText(const std::string& line)
	{
		if (isEnabled)
		{
			textLines.push_back(line);
		}
	}

	std::size_t DebugDraw::GetQueuedShapesCount() const
	{
		return circles.size() + rects.size() + lines.size();
	}

	void DebugDraw::Render()
	{
		float frameTime = FrameClock::Instance()->GetDeltaTime();
		smoothedFrameTime += (frameTime - smoothedFrameTime) * FRAME_TIME_SMOOTHING;

		if (!isEnabled)
		{
			return;
		}

		RenderColliders();
		RenderShapes();
		RenderText();
		Clear();
	}

	void DebugDraw::RenderColliders()
	{
		for (auto collider : PhysicsSystem::Instance()->GetColliders())
		{
			const sf::FloatRect& bounds = collider->GetBounds();

			sf::RectangleShape shape({bounds.width, bounds.height});
			shape.setPosition(bounds.left, bounds.top);
			shape.setFillColor(sf::Color::Transparent);
			shape.setOutlineColor(collider->IsTrigger() ? DEBUG_TRIGGER_COLOR : DEBUG_COLLIDER_COLOR);
			shape.setOutlineThickness(DEBUG_OUTLINE_THICKNESS);

			RenderSystem::Instance()->Render(shape);
		}
	}

	void DebugDraw::RenderShapes()
	{
		for (const Rect& rect : rects)
		{
			sf::RectangleShape shape({rect.rect.width, rect.rect.height});
			shape.setPosition(rect.rect.left, rect.rect.top);
			shape.setFillColor(sf::Color::Transparent);
			shape.setOutlineColor(rect.color);
			shape.setOutlineThickness(DEBUG_OUTLINE_THICKNESS);

			RenderSystem::Instance()->Render(shape);
		}

		for (const Circle& circle : circles)
		{
			sf::CircleShape shape(circle.radius);
			shape.setOrigin(circle.radius, circle.radius);
			shape.setPosition(circle.center.x, circle.center.y);
			shape.setFillColor(sf::Color::Transparent);
			shape.setOutlineColor(circle.color);
			shape.setOutlineThickness(DEBUG_OUTLINE_THICKNESS);

			RenderSystem::Instance()->Render(shape);
		}

		for (const Line& line : lines)
		{
			sf::Vertex vertices[2] = {
				sf::Vertex({line.from.x, line.from.y}, line.color),
				sf::Vertex({line.to.x, line.to.y}, line.color)
			};

			RenderSystem::Instance()->GetMainWindow().draw(vertices, 2, sf::Lines);
		}
	}

	void DebugDraw::RenderText()
	{
		if (font == nullptr)
		{
			return;
		}

		auto& window = RenderSystem::Instance()->GetMainWindow();
		sf::View worldView = window.getView();
		window.setView(RenderSystem::Instance()->GetUiView());

		float fps = smoothedFrameTime > 0.f ? 1.f / smoothedFrameTime : 0.f;

		char header[96];
		std::snprintf(header, sizeof(header), "%.0f fps  %.1f ms  frame %u", fps, smoothedFrameTime * 1000.f, FrameClock::Instance()->GetFrame());

		char counts[96];
		std::snprintf(counts, sizeof(counts), "objects %zu  colliders %zu",
			GameWorld::Instance()->GetObjectsCount(), PhysicsSystem::Instance()->GetColliders().size());

		sf::Text text;
		text.setFont(*font);
		text.setCharacterSize(DEBUG_TEXT_SIZE);
		text.setFillColor(DEBUG_TEXT_COLOR);
		text.setOutlineColor(DEBUG_TEXT_OUTLINE_COLOR);
		text.setOutlineThickness(DEBUG_TEXT_OUTLINE);

		float y = DEBUG_TEXT_MARGIN;
		auto drawLine = [&](const std::string& line)
		{
			text.setString(line);
			text.setPosition(DEBUG_TEXT_MARGIN, y);
			RenderSystem::Instance()->Render(text);
			y += DEBUG_TEXT_LINE_HEIGHT;
		};

		drawLine(header);
		drawLine(counts);
		for (const std::string& line : textLines)
		{
			drawLine(line);
		}

		window.setView(worldView);
	}

	void DebugDraw::Clear()
	{
		circles.clear();
		rects.clear();
		lines.clear();
		textLines.clear();
	}
}
