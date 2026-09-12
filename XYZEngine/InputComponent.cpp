#include "pch.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "RenderSystem.h"

namespace XYZEngine
{
	InputBindings GetDefaultBindings()
	{
		InputBindings bindings;
		bindings[static_cast<int>(InputAction::MoveUp)].key = sf::Keyboard::W;
		bindings[static_cast<int>(InputAction::MoveDown)].key = sf::Keyboard::S;
		bindings[static_cast<int>(InputAction::MoveLeft)].key = sf::Keyboard::A;
		bindings[static_cast<int>(InputAction::MoveRight)].key = sf::Keyboard::D;
		bindings[static_cast<int>(InputAction::Attack)].button = sf::Mouse::Left;
		bindings[static_cast<int>(InputAction::HeavyAttack)].button = sf::Mouse::Right;
		bindings[static_cast<int>(InputAction::Run)].key = sf::Keyboard::LShift;
		bindings[static_cast<int>(InputAction::Run)].alternativeKey = sf::Keyboard::RShift;
		bindings[static_cast<int>(InputAction::Reload)].key = sf::Keyboard::R;
		bindings[static_cast<int>(InputAction::Roll)].key = sf::Keyboard::Space;

		return bindings;
	}

	InputComponent::InputComponent(GameObject* gameObject) : Component(gameObject) {}

	void InputComponent::Update(float deltaTime)
	{
		auto input = InputSystem::Instance();

		for (int action = 0; action < INPUT_ACTIONS_COUNT; action++)
		{
			const InputBinding& binding = bindings[action];

			heldActions[action] = input->IsKeyHeld(binding.key) || input->IsKeyHeld(binding.alternativeKey)
				|| input->IsButtonHeld(binding.button);
			pressedActions[action] = input->WasKeyPressed(binding.key) || input->WasKeyPressed(binding.alternativeKey)
				|| input->WasButtonPressed(binding.button);
		}

		horizontalAxis = AxisValue(IsActionHeld(InputAction::MoveRight), IsActionHeld(InputAction::MoveLeft));
		verticalAxis = AxisValue(IsActionHeld(InputAction::MoveUp), IsActionHeld(InputAction::MoveDown));

		auto& window = RenderSystem::Instance()->GetMainWindow();
		auto worldPosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		mouseWorldPosition = Convert<Vector2Df, sf::Vector2f>(worldPosition);
	}
	void InputComponent::Render()
	{
	}

	void InputComponent::SetBinding(InputAction action, const InputBinding& binding)
	{
		bindings[static_cast<int>(action)] = binding;
	}
	const InputBinding& InputComponent::GetBinding(InputAction action) const
	{
		return bindings[static_cast<int>(action)];
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
		return heldActions[static_cast<int>(action)];
	}
	bool InputComponent::WasActionPressed(InputAction action) const
	{
		return pressedActions[static_cast<int>(action)];
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
