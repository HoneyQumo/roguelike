#pragma once

#include "Component.h"
#include "InputSystem.h"
#include "Vector.h"

namespace XYZEngine
{
	class InputComponent : public Component
	{
	public:
		InputComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		float GetHorizontalAxis() const;
		float GetVerticalAxis() const;

		bool IsActionHeld(InputAction action) const;
		bool WasActionPressed(InputAction action) const;

		Vector2Df GetMouseWorldPosition() const;

	private:
		float horizontalAxis = 0.f;
		float verticalAxis = 0.f;
		Vector2Df mouseWorldPosition = {0.f, 0.f};

		static float AxisValue(bool isPositiveHeld, bool isNegativeHeld);
	};
}
