#pragma once

#include <functional>
#include <vector>
#include "Component.h"

namespace XYZEngine
{
	class HealthComponent : public Component
	{
	public:
		HealthComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		void SetMaxHealth(float newMaxHealth);
		float GetMaxHealth() const;
		float GetHealth() const;
		float GetHealthPercent() const;

		void SetArmor(float newArmor);
		float GetArmor() const;

		void TakeDamage(float damage);
		void Heal(float amount);

		bool IsAlive() const;

		void SubscribeDamage(std::function<void(float)> onDamageAction);
		void SubscribeDeath(std::function<void()> onDeathAction);
	private:
		float maxHealth = 100.f;
		float health = 100.f;
		float armor = 0.f;

		std::vector<std::function<void(float)>> onDamageActions;
		std::vector<std::function<void()>> onDeathActions;

		float CalculateDamage(float damage) const;
	};
}
