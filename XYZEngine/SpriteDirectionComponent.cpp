#include "pch.h"
#include "SpriteDirectionComponent.h"

namespace XYZEngine
{
	const float MIN_DIRECTION = 0.01f;

	SpriteDirectionComponent::SpriteDirectionComponent(GameObject* gameObject) : Component(gameObject) {}

	// Direction comes from the movement intent, not from the actual offset:
	// physics pushes a blocked object backwards, and the sprite would flip away from the obstacle.
	void SpriteDirectionComponent::Update(float deltaTime)
	{
		if (movement == nullptr)
		{
			movement = gameObject->GetComponent<MovementComponent>();
		}
		if (renderer == nullptr)
		{
			renderer = gameObject->GetComponent<SpriteRendererComponent>();
		}

		if (movement == nullptr || renderer == nullptr)
		{
			return;
		}

		float directionX = movement->GetDirection().x;

		if (directionX < -MIN_DIRECTION)
		{
			renderer->FlipX(true);
		}
		else if (directionX > MIN_DIRECTION)
		{
			renderer->FlipX(false);
		}
	}
	void SpriteDirectionComponent::Render()
	{

	}
}
