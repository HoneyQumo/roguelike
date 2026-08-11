#pragma once

#include <functional>
#include "Component.h"
#include "TransformComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	class WeaponComponent : public Component
	{
	public:
		WeaponComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		void SetCooldown(float newCooldown);
		void SetDamage(float newDamage);
		void SetProjectileSpeed(float newProjectileSpeed);
		void SetShotOffset(float newShotOffset);
		void SetShotAction(std::function<void(const Vector2Df&, const Vector2Df&, float, float)> newShotAction);

		bool IsReady() const;
		bool TryShoot(const Vector2Df& direction);
	private:
		TransformComponent* transform;

		float cooldown = 0.5f;
		float cooldownTimer = 0.f;
		float damage = 10.f;
		float projectileSpeed = 600.f;
		float shotOffset = 40.f;

		std::function<void(const Vector2Df&, const Vector2Df&, float, float)> shotAction;
	};
}
