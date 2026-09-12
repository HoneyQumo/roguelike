#include "pch.h"
#include "VertexArrayRendererComponent.h"
#include "GameObject.h"
#include "RenderSystem.h"

namespace XYZEngine
{
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

		RenderSystem::Instance()->Render(vertices);
	}

	void VertexArrayRendererComponent::Clear()
	{
		vertices.clear();
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
	}

	std::size_t VertexArrayRendererComponent::GetQuadsCount() const
	{
		return vertices.getVertexCount() / 4;
	}
}
