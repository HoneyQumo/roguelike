#include "pch.h"
#include "SpriteDirectionComponent.h"

namespace XYZEngine
{
	const float MIN_OFFSET = 0.01f;

	SpriteDirectionComponent::SpriteDirectionComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();
		previousPosition = transform->GetWorldPosition();
	}

	void SpriteDirectionComponent::Update(float deltaTime)
	{
		if (renderer == nullptr)
		{
			renderer = gameObject->GetComponent<SpriteRendererComponent>();
			if (renderer == nullptr)
			{
				return;
			}
		}

		Vector2Df position = transform->GetWorldPosition();
		float offsetX = position.x - previousPosition.x;
		previousPosition = position;

		if (offsetX < -MIN_OFFSET)
		{
			renderer->FlipX(true);
		}
		else if (offsetX > MIN_OFFSET)
		{
			renderer->FlipX(false);
		}
	}
	void SpriteDirectionComponent::Render()
	{

	}
}
