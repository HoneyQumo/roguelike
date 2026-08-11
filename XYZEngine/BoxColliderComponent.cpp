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

		float centerX = position.x + offset.x * worldScale.x;
		float centerY = position.y + offset.y * worldScale.y;

		bounds = sf::FloatRect(centerX - 0.5f * width, centerY - 0.5f * height, width, height);
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

	void BoxColliderComponent::SetOffset(float offsetX, float offsetY)
	{
		offset = { offsetX, offsetY };
	}
	Vector2Df BoxColliderComponent::GetOffset() const
	{
		return offset;
	}
}
