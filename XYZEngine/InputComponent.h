#pragma once

#include <array>
#include "Component.h"
#include "Vector.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

namespace XYZEngine
{
	enum class InputAction
	{
		MoveUp,
		MoveDown,
		MoveLeft,
		MoveRight,
		Attack,
		HeavyAttack,
		Run,
		Reload,
		Roll,
		Count
	};

	constexpr int INPUT_ACTIONS_COUNT = static_cast<int>(InputAction::Count);

	struct InputBinding
	{
		sf::Keyboard::Key key = sf::Keyboard::Unknown;
		sf::Keyboard::Key alternativeKey = sf::Keyboard::Unknown;
		sf::Mouse::Button button = sf::Mouse::ButtonCount;
	};

	using InputBindings = std::array<InputBinding, INPUT_ACTIONS_COUNT>;

	InputBindings GetDefaultBindings();

	class InputComponent : public Component
	{
	public:
		InputComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		void SetBinding(InputAction action, const InputBinding& binding);
		const InputBinding& GetBinding(InputAction action) const;

		float GetHorizontalAxis() const;
		float GetVerticalAxis() const;

		bool IsActionHeld(InputAction action) const;
		bool WasActionPressed(InputAction action) const;

		Vector2Df GetMouseWorldPosition() const;

	private:
		InputBindings bindings = GetDefaultBindings();

		std::array<bool, INPUT_ACTIONS_COUNT> heldActions = {};
		std::array<bool, INPUT_ACTIONS_COUNT> pressedActions = {};

		float horizontalAxis = 0.f;
		float verticalAxis = 0.f;
		Vector2Df mouseWorldPosition = {0.f, 0.f};

		static float AxisValue(bool isPositiveHeld, bool isNegativeHeld);
	};
}
