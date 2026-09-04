#include "ProjectileComponent.h"
#include <GameObject.h>
#include <GameWorld.h>
#include "HealthComponent.h"
#include <PhysicsSystem.h>
#include <LoggerRegistry.h>
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace XYZEngine;

namespace RoguelikeGame
{
	ProjectileComponent::ProjectileComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetTransform();
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

		collider->SubscribeTriggerEnter([this](const Trigger& trigger) { OnTrigger(trigger); });
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
			expireEvent.Invoke(transform->GetWorldPosition());

			Destroy();
			return;
		}

		if (direction.IsZero())
		{
			Destroy();
			return;
		}

		Vector2Df move = speed * deltaTime * direction.Normalized();
		int steps = std::max(1, static_cast<int>(std::ceil(move.GetLength() / GetMaxStep())));
		Vector2Df step = (1.f / steps) * move;
		Vector2Df moved = {0.f, 0.f};

		for (int i = 0; i < steps; i++)
		{
			transform->MoveBy(step);
			moved = moved + step;

			ColliderComponent* target = FindHit(moved);
			if (target != nullptr)
			{
				Hit(target);
				return;
			}
		}
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
	SubscriptionId ProjectileComponent::SubscribeHit(std::function<void(const Vector2Df&, const Vector2Df&, bool)> onHit)
	{
		return hitEvent.Subscribe(std::move(onHit));
	}
	SubscriptionId ProjectileComponent::SubscribeExpire(std::function<void(const Vector2Df&)> onExpire)
	{
		return expireEvent.Subscribe(std::move(onExpire));
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

		if (otherCollider->GetGameObject()->GetName() == shooterName)
		{
			return;
		}

		Hit(otherCollider);
	}

	float ProjectileComponent::GetMaxStep() const
	{
		const sf::FloatRect& bounds = collider->GetBounds();
		return std::max(1.f, 0.5f * std::min(bounds.width, bounds.height));
	}

	ColliderComponent* ProjectileComponent::FindHit(const Vector2Df& moved) const
	{
		sf::FloatRect area = collider->GetBounds();
		area.left += moved.x;
		area.top += moved.y;

		for (ColliderComponent* other : PhysicsSystem::Instance()->Overlap(area))
		{
			bool isIgnored = (collider->GetCollisionLayer() & other->GetIgnoredLayers()) != 0u
				|| (other->GetCollisionLayer() & collider->GetIgnoredLayers()) != 0u;
			if (other == collider || other->IsTrigger() || isIgnored || other->GetGameObject()->GetName() == shooterName)
			{
				continue;
			}

			return other;
		}

		return nullptr;
	}

	void ProjectileComponent::Hit(ColliderComponent* target)
	{
		auto health = target->GetGameObject()->GetComponent<HealthComponent>();
		bool isCharacterHit = health != nullptr && health->IsAlive() && !health->IsInvulnerable();
		if (isCharacterHit && damage > 0.f)
		{
			health->TakeDamage(damage);
		}

		hitEvent.Invoke(transform->GetWorldPosition(), direction.Normalized(), isCharacterHit);

		Destroy();
	}

	void ProjectileComponent::Destroy()
	{
		isHandled = true;
		GameWorld::Instance()->DestroyGameObject(gameObject);
	}
}
