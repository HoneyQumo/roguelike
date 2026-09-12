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

		void Start() override;
		void Update(float deltaTime) override;
		void Render() override;

		void SetSpeed(float newSpeed);
		float GetSpeed() const;

		void SetRunSpeedMultiplier(float newRunSpeedMultiplier);
		bool IsRunning() const;

		void SetRunAllowed(bool newIsRunAllowed);
		bool IsRunAllowed() const;

		void SetDirection(const Vector2Df& newDirection);
		Vector2Df GetDirection() const;
	protected:
		void OnDisable() override;
	private:
		TransformComponent* transform = nullptr;
		InputComponent* input = nullptr;

		float speed = 0.f;
		float runSpeedMultiplier = 1.f;
		bool isRunning = false;
		bool isRunAllowed = true;
		Vector2Df direction = { 0.f, 0.f };
	};
}
