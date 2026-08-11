#include "pch.h"
#include "WeaponComponent.h"
#include "GameObject.h"
#include "LoggerRegistry.h"
#include <cassert>

namespace XYZEngine
{
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
	void WeaponComponent::SetShotOffset(float newShotOffset)
	{
		shotOffset = newShotOffset;
	}
	void WeaponComponent::SetShotAction(std::function<void(const Vector2Df&, const Vector2Df&, float, float)> newShotAction)
	{
		shotAction = newShotAction;
	}

	bool WeaponComponent::IsReady() const
	{
		return cooldownTimer <= 0.f;
	}

	bool WeaponComponent::TryShoot(const Vector2Df& direction)
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

		float length = direction.GetLength();
		if (length <= 0.f)
		{
			LOG_WARN("Weapon can't shoot without direction on " + gameObject->GetName());
			return false;
		}

		Vector2Df normalizedDirection = (1.f / length) * direction;
		Vector2Df shotPosition = transform->GetWorldPosition() + shotOffset * normalizedDirection;

		shotAction(shotPosition, normalizedDirection, damage, projectileSpeed);
		cooldownTimer = cooldown;

		LOG_INFO(gameObject->GetName() + " shoots");
		return true;
	}
}
