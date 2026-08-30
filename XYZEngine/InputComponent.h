#pragma once

#include "Component.h"
#include "Vector.h"
#include <SFML/Window.hpp>

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

		bool IsAttackPressed() const;
		bool IsRunPressed() const;
		bool IsReloadPressed() const;
		Vector2Df GetMouseWorldPosition() const;
	private:
		float horizontalAxis = 0.f;
		float verticalAxis = 0.f;
		bool isAttackPressed = false;
		bool isRunPressed = false;
		bool isReloadPressed = false;
		Vector2Df mouseWorldPosition = { 0.f, 0.f };
	};
}