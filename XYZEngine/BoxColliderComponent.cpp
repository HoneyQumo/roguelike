#include "pch.h"
#include "BoxColliderComponent.h"

namespace XYZEngine
{
	BoxColliderComponent::BoxColliderComponent(GameObject* gameObject) : ColliderComponent(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();
		PhysicsSystem::Instance()->Subscribe(this);
	}
	BoxColliderComponent::~BoxColliderComponent()
	{
		PhysicsSystem::Instance()->Unsubscribe(this);
	}

	void BoxColliderComponent::Update(float deltaTime)
	{
		auto position = transform->GetWorldPosition();
		auto worldScale = transform->GetWorldScale();

		float width = size.x * worldScale.x;
		float height = size.y * worldScale.y;

		bounds = sf::FloatRect(position.x - 0.5f * width, position.y - 0.5f * height, width, height);
	}
	void BoxColliderComponent::Render()
	{

	}

	void BoxColliderComponent::SetSize(float newWidth, float newHeight)
	{
		size = { newWidth, newHeight };
	}
	Vector2Df BoxColliderComponent::GetSize() const
	{
		return size;
	}
}
