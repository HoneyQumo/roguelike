#include "pch.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "RenderSystem.h"

namespace XYZEngine
{
	InputComponent::InputComponent(GameObject* gameObject) : Component(gameObject) {}

	void InputComponent::Update(float deltaTime)
	{
		horizontalAxis = AxisValue(IsActionHeld(InputAction::MoveRight), IsActionHeld(InputAction::MoveLeft));
		verticalAxis = AxisValue(IsActionHeld(InputAction::MoveUp), IsActionHeld(InputAction::MoveDown));

		auto& window = RenderSystem::Instance()->GetMainWindow();
		auto worldPosition = window.mapPixelToCoords(InputSystem::Instance()->GetMousePosition());
		mouseWorldPosition = Convert<Vector2Df, sf::Vector2f>(worldPosition);
	}
	void InputComponent::Render()
	{
	}

	float InputComponent::GetHorizontalAxis() const
	{
		return horizontalAxis;
	}
	float InputComponent::GetVerticalAxis() const
	{
		return verticalAxis;
	}

	bool InputComponent::IsActionHeld(InputAction action) const
	{
		return InputSystem::Instance()->IsActionHeld(action);
	}
	bool InputComponent::WasActionPressed(InputAction action) const
	{
		return InputSystem::Instance()->WasActionPressed(action);
	}

	Vector2Df InputComponent::GetMouseWorldPosition() const
	{
		return mouseWorldPosition;
	}

	float InputComponent::AxisValue(bool isPositiveHeld, bool isNegativeHeld)
	{
		return (isPositiveHeld ? 1.f : 0.f) - (isNegativeHeld ? 1.f : 0.f);
	}
}
