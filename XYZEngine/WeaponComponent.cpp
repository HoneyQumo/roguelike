#include "pch.h"
#include "WeaponComponent.h"
#include "GameObject.h"
#include "LoggerRegistry.h"
#include <cassert>

namespace XYZEngine
{
	const float MIN_AIM_CORRECTION_DISTANCE = 64.f;

	WeaponComponent::WeaponComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();
	}

	void WeaponComponent::Update(float deltaTime)
	{
		if (cooldownTimer > 0.f)
		{
			cooldownTimer -= deltaTime;
		}
	}
	void WeaponComponent::Render()
	{

	}

	void WeaponComponent::SetCooldown(float newCooldown)
	{
		assert(newCooldown >= 0.f);
		cooldown = newCooldown;
	}
	void WeaponComponent::SetDamage(float newDamage)
	{
		assert(newDamage >= 0.f);
		damage = newDamage;
	}
	void WeaponComponent::SetProjectileSpeed(float newProjectileSpeed)
	{
		assert(newProjectileSpeed > 0.f);
		projectileSpeed = newProjectileSpeed;
	}
	void WeaponComponent::SetMuzzleOffset(const Vector2Df& newMuzzleOffset)
	{
		muzzleOffset = newMuzzleOffset;
	}
	void WeaponComponent::SetShotAction(std::function<void(const Vector2Df&, const Vector2Df&, float, float)> newShotAction)
	{
		shotAction = newShotAction;
	}

	bool WeaponComponent::IsReady() const
	{
		return cooldownTimer <= 0.f;
	}

	bool WeaponComponent::TryShootAt(const Vector2Df& targetPosition)
	{
		if (!IsReady())
		{
			return false;
		}

		if (shotAction == nullptr)
		{
			LOG_ERROR("Weapon has no shot action on " + gameObject->GetName());
			return false;
		}

		Vector2Df ownerPosition = transform->GetWorldPosition();
		Vector2Df toTarget = targetPosition - ownerPosition;
		float distance = toTarget.GetLength();
		if (distance <= 0.f)
		{
			LOG_WARN("Weapon can't shoot at its own position on " + gameObject->GetName());
			return false;
		}

		Vector2Df aimDirection = (1.f / distance) * toTarget;
		Vector2Df sideDirection = { -aimDirection.y, aimDirection.x };
		Vector2Df shotPosition = ownerPosition + muzzleOffset.x * aimDirection + muzzleOffset.y * sideDirection;

		Vector2Df shotDirection = aimDirection;
		if (distance > MIN_AIM_CORRECTION_DISTANCE)
		{
			Vector2Df fromMuzzle = targetPosition - shotPosition;
			float muzzleDistance = fromMuzzle.GetLength();
			if (muzzleDistance > 0.f)
			{
				shotDirection = (1.f / muzzleDistance) * fromMuzzle;
			}
		}

		shotAction(shotPosition, shotDirection, damage, projectileSpeed);
		cooldownTimer = cooldown;

		LOG_INFO(gameObject->GetName() + " shoots");
		return true;
	}
}
