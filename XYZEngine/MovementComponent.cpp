#include "pch.h"
#include "MovementComponent.h"
#include "GameObject.h"

namespace XYZEngine
{
	MovementComponent::MovementComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetTransform();
	}

	void MovementComponent::Start()
	{
		input = gameObject->GetComponent<InputComponent>();
	}

	void MovementComponent::Update(float deltaTime)
	{
		if (!isEnabled)
		{
			isRunning = false;
			return;
		}

		// If the owner is player-controlled, input overrides any direction set from the outside.
		if (input != nullptr)
		{
			direction = { input->GetHorizontalAxis(), input->GetVerticalAxis() };
		}

		isRunning = false;

		if (direction.IsZero() || speed <= 0.f)
		{
			return;
		}

		isRunning = input != nullptr && input->IsRunPressed();
		float currentSpeed = isRunning ? speed * runSpeedMultiplier : speed;

		transform->MoveBy(currentSpeed * deltaTime * direction.Normalized());
	}
	void MovementComponent::Render()
	{

	}

	void MovementComponent::SetEnabled(bool newIsEnabled)
	{
		isEnabled = newIsEnabled;
	}
	bool MovementComponent::IsEnabled() const
	{
		return isEnabled;
	}

	void MovementComponent::SetSpeed(float newSpeed)
	{
		speed = newSpeed;
	}
	float MovementComponent::GetSpeed() const
	{
		return speed;
	}

	void MovementComponent::SetRunSpeedMultiplier(float newRunSpeedMultiplier)
	{
		runSpeedMultiplier = newRunSpeedMultiplier;
	}
	bool MovementComponent::IsRunning() const
	{
		return isRunning;
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
