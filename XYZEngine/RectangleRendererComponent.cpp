#include "pch.h"
#include "RectangleRendererComponent.h"
#include "RenderSystem.h"

namespace XYZEngine
{
	RectangleRendererComponent::RectangleRendererComponent(GameObject* gameObject) : Component(gameObject)
	{
		rectangle = new sf::RectangleShape();
		transform = gameObject->GetComponent<TransformComponent>();
	}
	RectangleRendererComponent::~RectangleRendererComponent()
	{
		delete rectangle;
	}

	void RectangleRendererComponent::Update(float deltaTime)
	{

	}
	void RectangleRendererComponent::Render()
	{
		auto worldScale = transform->GetWorldScale();
		sf::Vector2f scaledSize = { size.x * worldScale.x, size.y * worldScale.y };

		rectangle->setSize(scaledSize);
		rectangle->setOrigin(0.5f * scaledSize.x, 0.5f * scaledSize.y);
		rectangle->setPosition(Convert<sf::Vector2f, Vector2Df>(transform->GetWorldPosition()));
		rectangle->setRotation(transform->GetWorldRotation());

		RenderSystem::Instance()->Render(*rectangle);
	}

	void RectangleRendererComponent::SetSize(float newWidth, float newHeight)
	{
		size = { newWidth, newHeight };
	}
	Vector2Df RectangleRendererComponent::GetSize() const
	{
		return size;
	}

	void RectangleRendererComponent::SetColor(const sf::Color& newColor)
	{
		rectangle->setFillColor(newColor);
	}
}
