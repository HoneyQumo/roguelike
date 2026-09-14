#pragma once

#include <string>
#include "Component.h"
#include "TransformComponent.h"
#include "InputComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	// Поворачивает объект целиком на прицел вместо отражения спрайта: кадр вида сверху читается под любым углом.
	class AimRotationComponent : public Component
	{
	public:
		AimRotationComponent(GameObject* gameObject);

		void Start() override;
		void Update(float deltaTime) override;
		void Render() override;

		void AimAtCursor();
		void AimAtGameObject(const std::string& newTargetName);
		void AimAtPoint(const Vector2Df& point);
		void StopAiming();
		void SetMaxDistance(float newMaxDistance);

		const Vector2Df& GetAimDirection() const;
	private:
		TransformComponent* transform = nullptr;
		InputComponent* input = nullptr;

		std::string targetName;
		Vector2Df aimPoint = { 0.f, 0.f };
		bool isCursorAim = false;
		bool isPointAim = false;
		float maxDistance = 0.f;

		Vector2Df aimDirection = { 1.f, 0.f };

		bool TryGetAimPosition(Vector2Df& aimPosition);
	};
}
