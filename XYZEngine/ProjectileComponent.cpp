#include "pch.h"
#include "ProjectileComponent.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "HealthComponent.h"
#include "LoggerRegistry.h"
#include <cassert>

namespace XYZEngine
{
	ProjectileComponent::ProjectileComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();
	}

	void ProjectileComponent::Update(float deltaTime)
	{
		if (isHandled)
		{
			return;
		}

		if (collider == nullptr)
		{
			collider = gameObject->GetComponent<ColliderComponent>();
			if (collider == nullptr)
			{
				LOG_ERROR("Projectile needs a collider on " + gameObject->GetName());
				Destroy();
				return;
			}

			collider->SubscribeTriggerEnter([this](Trigger trigger) { OnTrigger(trigger); });
		}

		lifetime -= deltaTime;
		if (lifetime <= 0.f)
		{
			Destroy();
			return;
		}

		float length = direction.GetLength();
		if (length <= 0.f)
		{
			Destroy();
			return;
		}

		Vector2Df normalizedDirection = (1.f / length) * direction;
		transform->MoveBy(speed * deltaTime * normalizedDirection);
	}
	void ProjectileComponent::Render()
	{

	}

	void ProjectileComponent::SetDirection(const Vector2Df& newDirection)
	{
		direction = newDirection;
	}
	void ProjectileComponent::SetSpeed(float newSpeed)
	{
		assert(newSpeed > 0.f);
		speed = newSpeed;
	}
	void ProjectileComponent::SetDamage(float newDamage)
	{
		assert(newDamage >= 0.f);
		damage = newDamage;
	}
	void ProjectileComponent::SetLifetime(float newLifetime)
	{
		assert(newLifetime > 0.f);
		lifetime = newLifetime;
	}
	void ProjectileComponent::SetShooterName(const std::string& newShooterName)
	{
		shooterName = newShooterName;
	}

	void ProjectileComponent::OnTrigger(const Trigger& trigger)
	{
		if (isHandled)
		{
			return;
		}

		ColliderComponent* otherCollider = trigger.GetFirst();
		if (otherCollider == collider)
		{
			otherCollider = trigger.GetSecond();
		}

		if (otherCollider == nullptr)
		{
			return;
		}

		GameObject* target = otherCollider->GetGameObject();
		if (target->GetName() == shooterName)
		{
			return;
		}

		auto health = target->GetComponent<HealthComponent>();
		if (health != nullptr && health->IsAlive())
		{
			health->TakeDamage(damage);
		}

		Destroy();
	}

	void ProjectileComponent::Destroy()
	{
		isHandled = true;
		GameWorld::Instance()->DestroyGameObject(gameObject);
	}
}
