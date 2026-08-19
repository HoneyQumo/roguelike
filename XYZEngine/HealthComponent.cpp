#include "pch.h"
#include "HealthComponent.h"
#include "GameObject.h"
#include "LoggerRegistry.h"
#include <cassert>

namespace XYZEngine
{
	const float MIN_DAMAGE = 1.f;
	const float LOW_HEALTH_PERCENT = 0.3f;

	HealthComponent::HealthComponent(GameObject* gameObject) : Component(gameObject) {}

	void HealthComponent::Update(float deltaTime)
	{

	}
	void HealthComponent::Render()
	{

	}

	void HealthComponent::SetMaxHealth(float newMaxHealth)
	{
		assert(newMaxHealth > 0.f);

		if (newMaxHealth <= 0.f)
		{
			LOG_WARN("Max health must be positive on " + gameObject->GetName());
			return;
		}

		maxHealth = newMaxHealth;
		health = newMaxHealth;
	}
	float HealthComponent::GetMaxHealth() const
	{
		return maxHealth;
	}
	float HealthComponent::GetHealth() const
	{
		return health;
	}
	float HealthComponent::GetHealthPercent() const
	{
		return health / maxHealth;
	}

	void HealthComponent::SetArmor(float newArmor)
	{
		assert(newArmor >= 0.f);

		if (newArmor < 0.f)
		{
			LOG_WARN("Armor can't be negative on " + gameObject->GetName());
			return;
		}

		armor = newArmor;
	}
	float HealthComponent::GetArmor() const
	{
		return armor;
	}

	void HealthComponent::TakeDamage(float damage)
	{
		assert(damage >= 0.f);

		if (damage < 0.f)
		{
			LOG_WARN("Negative damage is ignored on " + gameObject->GetName());
			return;
		}

		if (!IsAlive())
		{
			return;
		}

		float takenDamage = CalculateDamage(damage);
		health -= takenDamage;
		if (health < 0.f)
		{
			health = 0.f;
		}

		LOG_INFO(gameObject->GetName() + " takes " + std::to_string((int)takenDamage)
			+ " damage, health " + std::to_string((int)health) + "/" + std::to_string((int)maxHealth));

		for (auto& onDamageAction : onDamageActions)
		{
			onDamageAction(takenDamage);
		}

		if (!IsAlive())
		{
			LOG_WARN(gameObject->GetName() + " is dead");

			for (auto& onDeathAction : onDeathActions)
			{
				onDeathAction();
			}
			return;
		}

		if (GetHealthPercent() <= LOW_HEALTH_PERCENT)
		{
			LOG_WARN(gameObject->GetName() + " health is low: " + std::to_string((int)health));
		}
	}
	void HealthComponent::Heal(float amount)
	{
		assert(amount >= 0.f);

		if (amount < 0.f || !IsAlive())
		{
			return;
		}

		health += amount;
		if (health > maxHealth)
		{
			health = maxHealth;
		}

		LOG_INFO(gameObject->GetName() + " healed to " + std::to_string((int)health));
	}

	bool HealthComponent::IsAlive() const
	{
		return health > 0.f;
	}

	void HealthComponent::SubscribeDamage(std::function<void(float)> onDamageAction)
	{
		onDamageActions.push_back(onDamageAction);
	}
	void HealthComponent::SubscribeDeath(std::function<void()> onDeathAction)
	{
		onDeathActions.push_back(onDeathAction);
	}

	float HealthComponent::CalculateDamage(float damage) const
	{
		float reducedDamage = damage - armor;
		return reducedDamage < MIN_DAMAGE ? MIN_DAMAGE : reducedDamage;
	}
}
