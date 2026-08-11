#pragma once

#include "Component.h"
#include "TransformComponent.h"
#include "InputComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	class MovementComponent : public Component
	{
	public:
		MovementComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		void SetSpeed(float newSpeed);
		float GetSpeed() const;

		void SetDirection(const Vector2Df& newDirection);
		Vector2Df GetDirection() const;
	private:
		TransformComponent* transform;
		InputComponent* input = nullptr;

		float speed = 0.f;
		Vector2Df direction = { 0.f, 0.f };
	};
}
