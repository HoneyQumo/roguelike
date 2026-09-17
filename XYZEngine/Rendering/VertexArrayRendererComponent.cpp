#include "pch.h"
#include "VertexArrayRendererComponent.h"
#include "GameObject.h"
#include "RenderSystem.h"
#include "ViewCulling.h"

namespace XYZEngine
{
	constexpr float TEXTURE_EDGE_INSET = 0.5f;

	VertexArrayRendererComponent::VertexArrayRendererComponent(GameObject* gameObject) : Component(gameObject) {}

	void VertexArrayRendererComponent::Update(float deltaTime)
	{
	}

	void VertexArrayRendererComponent::Render()
	{
		if (vertices.getVertexCount() == 0)
		{
			return;
		}

		// Геометрия не разбирается поквадово: либо весь массив на экране, либо его нет.
		// Поэтому резать полотно на куски и имеет смысл.
		if (hasBounds && !RenderSystem::Instance()->IsVisible(bounds))
		{
			return;
		}

		RenderSystem::Instance()->CountVertices(static_cast<int>(vertices.getVertexCount()));

		if (texture != nullptr)
		{
			RenderSystem::Instance()->Render(vertices, sf::RenderStates(texture));
			return;
		}

		RenderSystem::Instance()->Render(vertices);
	}

	sf::FloatRect VertexArrayRendererComponent::GetBounds() const
	{
		return bounds;
	}

	void VertexArrayRendererComponent::Cover(const Vector2Df& point)
	{
		if (!hasBounds)
		{
			bounds = sf::FloatRect(point.x, point.y, 0.f, 0.f);
			hasBounds = true;
			return;
		}

		float left = std::min(bounds.left, point.x);
		float top = std::min(bounds.top, point.y);
		float right = std::max(bounds.left + bounds.width, point.x);
		float bottom = std::max(bounds.top + bounds.height, point.y);

		bounds = sf::FloatRect(left, top, right - left, bottom - top);
	}

	void VertexArrayRendererComponent::Clear()
	{
		vertices.clear();
		hasBounds = false;
		bounds = sf::FloatRect();
	}

	void VertexArrayRendererComponent::AddQuad(const Vector2Df& center, const Vector2Df& size, const sf::Color& color)
	{
		float left = center.x - 0.5f * size.x;
		float right = center.x + 0.5f * size.x;
		float bottom = center.y - 0.5f * size.y;
		float top = center.y + 0.5f * size.y;

		vertices.append({ { left, bottom }, color });
		vertices.append({ { right, bottom }, color });
		vertices.append({ { right, top }, color });
		vertices.append({ { left, top }, color });

		Cover({ left, bottom });
		Cover({ right, top });
	}

	void VertexArrayRendererComponent::AddQuad(const Vector2Df& first, const Vector2Df& second, const Vector2Df& third,
		const Vector2Df& fourth, const sf::Color& color)
	{
		vertices.append({ { first.x, first.y }, color });
		vertices.append({ { second.x, second.y }, color });
		vertices.append({ { third.x, third.y }, color });
		vertices.append({ { fourth.x, fourth.y }, color });

		Cover(first);
		Cover(second);
		Cover(third);
		Cover(fourth);
	}

	void VertexArrayRendererComponent::SetTexture(const sf::Texture* newTexture)
	{
		texture = newTexture;
	}

	void VertexArrayRendererComponent::AddQuad(const Vector2Df& center, const Vector2Df& size, const sf::IntRect& frame,
		const sf::Color& tint)
	{
		float left = center.x - 0.5f * size.x;
		float right = center.x + 0.5f * size.x;
		float bottom = center.y - 0.5f * size.y;
		float top = center.y + 0.5f * size.y;

		float u0 = static_cast<float>(frame.left) + TEXTURE_EDGE_INSET;
		float u1 = static_cast<float>(frame.left + frame.width) - TEXTURE_EDGE_INSET;
		float v0 = static_cast<float>(frame.top) + TEXTURE_EDGE_INSET;
		float v1 = static_cast<float>(frame.top + frame.height) - TEXTURE_EDGE_INSET;

		vertices.append({ { left, bottom }, tint, { u0, v1 } });
		vertices.append({ { right, bottom }, tint, { u1, v1 } });
		vertices.append({ { right, top }, tint, { u1, v0 } });
		vertices.append({ { left, top }, tint, { u0, v0 } });

		Cover({ left, bottom });
		Cover({ right, top });
	}

	void VertexArrayRendererComponent::SetQuadFrame(std::size_t quad, const sf::IntRect& frame)
	{
		std::size_t first = quad * 4u;
		if (first + 3u >= vertices.getVertexCount())
		{
			return;
		}

		float u0 = static_cast<float>(frame.left) + TEXTURE_EDGE_INSET;
		float u1 = static_cast<float>(frame.left + frame.width) - TEXTURE_EDGE_INSET;
		float v0 = static_cast<float>(frame.top) + TEXTURE_EDGE_INSET;
		float v1 = static_cast<float>(frame.top + frame.height) - TEXTURE_EDGE_INSET;

		vertices[first + 0u].texCoords = { u0, v1 };
		vertices[first + 1u].texCoords = { u1, v1 };
		vertices[first + 2u].texCoords = { u1, v0 };
		vertices[first + 3u].texCoords = { u0, v0 };
	}

	const sf::VertexArray& VertexArrayRendererComponent::GetVertices() const
	{
		return vertices;
	}

	std::size_t VertexArrayRendererComponent::GetQuadsCount() const
	{
		return vertices.getVertexCount() / 4;
	}
}
