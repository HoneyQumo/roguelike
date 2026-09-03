#include "pch.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "RenderSystem.h"

namespace XYZEngine
{
	InputComponent::InputComponent(GameObject* gameObject) : Component(gameObject) {}

	void InputComponent::Update(float deltaTime)
	{
		auto input = InputSystem::Instance();

		verticalAxis = 0.f;
		horizontalAxis = 0.f;

		if (input->IsKeyHeld(bindings.moveUp))
		{
			verticalAxis += 1.0f;
		}
		if (input->IsKeyHeld(bindings.moveDown))
		{
			verticalAxis -= 1.0f;
		}
		if (input->IsKeyHeld(bindings.moveRight))
		{
			horizontalAxis += 1.0f;
		}
		if (input->IsKeyHeld(bindings.moveLeft))
		{
			horizontalAxis -= 1.0f;
		}

		isAttackPressed = input->IsButtonHeld(bindings.attack);
		isHeavyAttackPressed = input->IsButtonHeld(bindings.heavyAttack);
		isRunPressed = input->IsKeyHeld(bindings.run) || input->IsKeyHeld(bindings.runAlternative);
		isReloadPressed = input->IsKeyHeld(bindings.reload);
		isRollJustPressed = input->WasKeyPressed(bindings.roll);

		selectedWeaponSlot = NO_WEAPON_SLOT;
		for (int digit = 0; digit < DIGIT_KEYS_COUNT; digit++)
		{
			if (input->WasKeyPressed(bindings.digits[digit]))
			{
				selectedWeaponSlot = digit;
				break;
			}
		}

		auto& window = RenderSystem::Instance()->GetMainWindow();
		auto worldPosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		mouseWorldPosition = Convert<Vector2Df, sf::Vector2f>(worldPosition);
	}
	void InputComponent::Render()
	{
	}

	void InputComponent::SetBindings(const InputBindings& newBindings)
	{
		bindings = newBindings;
	}
	const InputBindings& InputComponent::GetBindings() const
	{
		return bindings;
	}

	float InputComponent::GetHorizontalAxis() const
	{
		return horizontalAxis;
	}
	float InputComponent::GetVerticalAxis() const
	{
		return verticalAxis;
	}

	bool InputComponent::IsAttackPressed() const
	{
		return isAttackPressed;
	}
	bool InputComponent::IsHeavyAttackPressed() const
	{
		return isHeavyAttackPressed;
	}
	bool InputComponent::IsRunPressed() const
	{
		return isRunPressed;
	}
	bool InputComponent::IsReloadPressed() const
	{
		return isReloadPressed;
	}
	bool InputComponent::WasRollJustPressed() const
	{
		return isRollJustPressed;
	}
	int InputComponent::GetSelectedWeaponSlot() const
	{
		return selectedWeaponSlot;
	}
	Vector2Df InputComponent::GetMouseWorldPosition() const
	{
		return mouseWorldPosition;
	}
}
