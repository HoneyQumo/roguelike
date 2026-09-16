#include "HealthComponent.h"
#include "ArmorRules.h"
#include <GameObject.h>
#include <TransformComponent.h>
#include <LoggerRegistry.h>
#include <cassert>

using namespace XYZEngine;

namespace RoguelikeGame
{
    constexpr float LOW_HEALTH_PERCENT = 0.3f;

    HealthComponent::HealthComponent(GameObject* gameObject) : Component(gameObject)
    {
    }

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

    void HealthComponent::SetMaxArmor(float newMaxArmor)
    {
        assert(newMaxArmor >= 0.f);

        if (newMaxArmor < 0.f)
        {
            LOG_WARN("Max armor can't be negative on " + gameObject->GetName());
            return;
        }

        maxArmor = newMaxArmor;
        armor = newMaxArmor;
    }

    float HealthComponent::GetMaxArmor() const
    {
        return maxArmor;
    }

    void HealthComponent::SetArmor(float newArmor)
    {
        assert(newArmor >= 0.f);

        if (newArmor < 0.f)
        {
            LOG_WARN("Armor can't be negative on " + gameObject->GetName());
            return;
        }

        if (newArmor > maxArmor)
        {
            maxArmor = newArmor;
        }

        armor = newArmor;
    }

    float HealthComponent::GetArmor() const
    {
        return armor;
    }

    float HealthComponent::GetArmorPercent() const
    {
        return maxArmor > 0.f ? armor / maxArmor : 0.f;
    }

    void HealthComponent::SetInvulnerable(bool newIsInvulnerable)
    {
        isInvulnerable = newIsInvulnerable;
    }

    bool HealthComponent::IsInvulnerable() const
    {
        return isInvulnerable;
    }

    void HealthComponent::TakeDamage(float damage, const DamageSource& source)
    {
        assert(damage >= 0.f);

        if (damage < 0.f)
        {
            LOG_WARN("Negative damage is ignored on " + gameObject->GetName());
            return;
        }

        if (isInvulnerable || !IsAlive())
        {
            return;
        }


        ArmorHit hit = SplitDamage(damage, armor, IgnoresArmor(source.kind) ? 0.f : ARMOR_ABSORB_SHARE);
        armor -= hit.toArmor;

        float takenDamage = hit.toHealth;
        health -= takenDamage;
        if (health < 0.f)
        {
            health = 0.f;
        }

        LOG_INFO(gameObject->GetName() + " takes " + std::to_string(static_cast<int>(takenDamage))
            + " damage, health " + std::to_string(static_cast<int>(health)) + "/" + std::to_string(static_cast<int>(maxHealth)));

        DamageInfo info;
        info.amount = takenDamage;
        info.rawAmount = damage;
        info.armorAmount = hit.toArmor;
        info.isLethal = !IsAlive();
        info.source = source;

        damageEvent.Invoke(info);

        if (!IsAlive())
        {
            LOG_WARN(gameObject->GetName() + " is dead, killed by "
                + (source.attackerName.empty() ? "nobody" : source.attackerName));

            auto transform = gameObject->GetTransform();

            DeathInfo death;
            death.position = transform->GetWorldPosition();
            death.rotation = transform->GetWorldRotation();
            death.source = source;

            deathEvent.Invoke(death);
            return;
        }

        if (GetHealthPercent() <= LOW_HEALTH_PERCENT)
        {
            LOG_WARN(gameObject->GetName() + " health is low: " + std::to_string(static_cast<int>(health)));
        }
    }

    float HealthComponent::Heal(float amount)
    {
        assert(amount >= 0.f);

        if (amount < 0.f || !IsAlive())
        {
            return 0.f;
        }

        float restored = maxHealth - health;
        if (restored > amount)
        {
            restored = amount;
        }

        if (restored <= 0.f)
        {
            return 0.f;
        }

        health += restored;

        LOG_INFO(gameObject->GetName() + " healed by " + std::to_string(static_cast<int>(restored))
            + " to " + std::to_string(static_cast<int>(health)));

        healEvent.Invoke(restored);

        return restored;
    }

    bool HealthComponent::IsAlive() const
    {
        return health > 0.f;
    }

    XYZEngine::SubscriptionId HealthComponent::SubscribeDamage(std::function<void(const DamageInfo&)> onDamage)
    {
        return damageEvent.Subscribe(std::move(onDamage));
    }
    void HealthComponent::UnsubscribeDamage(XYZEngine::SubscriptionId subscription)
    {
        damageEvent.Unsubscribe(subscription);
    }

    XYZEngine::SubscriptionId HealthComponent::SubscribeHeal(std::function<void(float)> onHeal)
    {
        return healEvent.Subscribe(std::move(onHeal));
    }
    void HealthComponent::UnsubscribeHeal(XYZEngine::SubscriptionId subscription)
    {
        healEvent.Unsubscribe(subscription);
    }

    XYZEngine::SubscriptionId HealthComponent::SubscribeDeath(std::function<void(const DeathInfo&)> onDeath)
    {
        return deathEvent.Subscribe(std::move(onDeath));
    }
    void HealthComponent::UnsubscribeDeath(XYZEngine::SubscriptionId subscription)
    {
        deathEvent.Unsubscribe(subscription);
    }
}
