#include "ProjectileComponent.h"
#include <GameObject.h>
#include <GameWorld.h>
#include "HealthComponent.h"
#include <LoggerRegistry.h>
#include <cassert>

using namespace XYZEngine;

namespace RoguelikeGame
{
	ProjectileComponent::ProjectileComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();
	}

	void ProjectileComponent::Start()
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

	void ProjectileComponent::Update(float deltaTime)
	{
		if (isHandled)
		{
			return;
		}

		if (collider == nullptr)
		{
			return;
		}

		lifetime.Tick(deltaTime);
		if (lifetime.IsReady())
		{
			if (expireAction != nullptr)
			{
				expireAction(transform->GetWorldPosition());
			}

			Destroy();
			return;
		}

		if (direction.IsZero())
		{
			Destroy();
			return;
		}

		transform->MoveBy(speed * deltaTime * direction.Normalized());
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
		lifetime.Start(newLifetime);
	}
	void ProjectileComponent::SetShooterName(const std::string& newShooterName)
	{
		shooterName = newShooterName;
	}
	void ProjectileComponent::SetHitAction(std::function<void(const Vector2Df&, const Vector2Df&, bool)> newHitAction)
	{
		hitAction = newHitAction;
	}
	void ProjectileComponent::SetExpireAction(std::function<void(const Vector2Df&)> newExpireAction)
	{
		expireAction = newExpireAction;
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
		bool isCharacterHit = health != nullptr && health->IsAlive() && !health->IsInvulnerable();
		if (isCharacterHit && damage > 0.f)
		{
			health->TakeDamage(damage);
		}

		if (hitAction != nullptr)
		{
			hitAction(transform->GetWorldPosition(), direction.Normalized(), isCharacterHit);
		}

		Destroy();
	}

	void ProjectileComponent::Destroy()
	{
		isHandled = true;
		GameWorld::Instance()->DestroyGameObject(gameObject);
	}
}
