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

		void Update(float deltaTime) override;
		void Render() override;

		void AimAtCursor();
		void AimAtGameObject(const std::string& newTargetName);
		void SetMaxDistance(float newMaxDistance);
		void SetEnabled(bool newIsEnabled);

		const Vector2Df& GetAimDirection() const;
	private:
		TransformComponent* transform;
		InputComponent* input = nullptr;

		std::string targetName;
		bool isCursorAim = false;
		bool isEnabled = true;
		float maxDistance = 0.f;

		Vector2Df aimDirection = { 1.f, 0.f };

		bool TryGetAimPosition(Vector2Df& aimPosition);
	};
}
