#pragma once

#include <functional>
#include <Component.h>
#include <EventList.h>
#include "DamageInfo.h"

namespace RoguelikeGame
{
    class HealthComponent : public XYZEngine::Component
    {
    public:
        HealthComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetMaxHealth(float newMaxHealth);
        float GetMaxHealth() const;
        float GetHealth() const;
        float GetHealthPercent() const;

        void SetArmor(float newArmor);
        float GetArmor() const;
        
        void SetInvulnerable(bool newIsInvulnerable);
        bool IsInvulnerable() const;

        void TakeDamage(float damage, const DamageSource& source = DamageSource());
        float Heal(float amount);

        bool IsAlive() const;

        XYZEngine::SubscriptionId SubscribeDamage(std::function<void(const DamageInfo&)> onDamage);
        void UnsubscribeDamage(XYZEngine::SubscriptionId subscription);

        XYZEngine::SubscriptionId SubscribeHeal(std::function<void(float)> onHeal);
        void UnsubscribeHeal(XYZEngine::SubscriptionId subscription);

        XYZEngine::SubscriptionId SubscribeDeath(std::function<void(const DeathInfo&)> onDeath);
        void UnsubscribeDeath(XYZEngine::SubscriptionId subscription);

    private:
        float maxHealth = 100.f;
        float health = 100.f;
        float armor = 0.f;
        bool isInvulnerable = false;

        XYZEngine::EventList<const DamageInfo&> damageEvent;
        XYZEngine::EventList<float> healEvent;
        XYZEngine::EventList<const DeathInfo&> deathEvent;

        float CalculateDamage(float damage) const;
    };
}
