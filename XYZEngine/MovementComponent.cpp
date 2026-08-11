#include "pch.h"
#include "MovementComponent.h"

namespace XYZEngine
{
	MovementComponent::MovementComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();
	}

	void MovementComponent::Update(float deltaTime)
	{
		// If the owner is player-controlled, input overrides any direction set from the outside.
		if (input == nullptr)
		{
			input = gameObject->GetComponent<InputComponent>();
		}

		if (input != nullptr)
		{
			direction = { input->GetHorizontalAxis(), input->GetVerticalAxis() };
		}

		float length = direction.GetLength();
		if (length <= 0.f)
		{
			return;
		}

		Vector2Df normalizedDirection = (1.f / length) * direction;
		transform->MoveBy(speed * deltaTime * normalizedDirection);
	}
	void MovementComponent::Render()
	{

	}

	void MovementComponent::SetSpeed(float newSpeed)
	{
		speed = newSpeed;
	}
	float MovementComponent::GetSpeed() const
	{
		return speed;
	}

	void MovementComponent::SetDirection(const Vector2Df& newDirection)
	{
		direction = newDirection;
	}
	Vector2Df MovementComponent::GetDirection() const
	{
		return direction;
	}
}
