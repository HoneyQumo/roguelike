#include "pch.h"
#include "SpriteDirectionComponent.h"
#include "GameObject.h"

namespace XYZEngine
{
	const float MIN_DIRECTION = 0.01f;

	SpriteDirectionComponent::SpriteDirectionComponent(GameObject* gameObject) : Component(gameObject) {}

	// Direction comes from the movement intent, not from the actual offset:
	// physics pushes a blocked object backwards, and the sprite would flip away from the obstacle.
	void SpriteDirectionComponent::Start()
	{
		movement = gameObject->GetComponent<MovementComponent>();
		renderer = gameObject->GetComponent<SpriteRendererComponent>();
	}

	void SpriteDirectionComponent::Update(float deltaTime)
	{
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
