#include "StaminaComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <MovementComponent.h>
#include <algorithm>

namespace RoguelikeGame
{
    StaminaComponent::StaminaComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void StaminaComponent::Start()
    {
        movement = gameObject->GetComponent<XYZEngine::MovementComponent>();
    }

    void StaminaComponent::SetMaxStamina(float newMaxStamina)
    {
        if (newMaxStamina <= 0.f)
        {
            LOG_WARN("Max stamina must be positive on " + gameObject->GetName());
            return;
        }

        maxStamina = newMaxStamina;
        stamina = newMaxStamina;
        isExhausted = false;

        Publish();
    }

    void StaminaComponent::SetRegen(float newRegenPerSecond, float newRegenDelay)
    {
        regenPerSecond = std::max(newRegenPerSecond, 0.f);
        regenDelay = std::max(newRegenDelay, 0.f);
    }

    void StaminaComponent::SetRunDrain(float newRunDrainPerSecond)
    {
        runDrainPerSecond = std::max(newRunDrainPerSecond, 0.f);
    }

    void StaminaComponent::SetRunResumePart(float newRunResumePart)
    {
        runResumePart = std::clamp(newRunResumePart, 0.f, 1.f);
    }

    float StaminaComponent::GetMaxStamina() const
    {
        return maxStamina;
    }

    float StaminaComponent::GetStamina() const
    {
        return stamina;
    }

    float StaminaComponent::GetStaminaPercent() const
    {
        return stamina / maxStamina;
    }

    bool StaminaComponent::IsExhausted() const
    {
        return isExhausted;
    }

    bool StaminaComponent::HasStamina(float amount) const
    {
        return stamina >= amount;
    }

    bool StaminaComponent::TrySpend(float amount)
    {
        if (amount <= 0.f)
        {
            return true;
        }

        if (!HasStamina(amount))
        {
            return false;
        }

        Spend(amount);
        return true;
    }

    void StaminaComponent::Spend(float amount)
    {
        stamina = std::max(stamina - amount, 0.f);
        regenTimer = regenDelay;

        if (stamina <= 0.f)
        {
            isExhausted = true;
        }

        Publish();
    }

    void StaminaComponent::Update(float deltaTime)
    {
        UpdateRun(deltaTime);
        UpdateRegen(deltaTime);
    }

    void StaminaComponent::UpdateRun(float deltaTime)
    {
        if (movement == nullptr)
        {
            return;
        }

        if (movement->IsRunning())
        {
            Spend(runDrainPerSecond * deltaTime);

            if (stamina <= 0.f)
            {
                movement->SetRunAllowed(false);
            }

            return;
        }

        if (isExhausted && GetStaminaPercent() >= runResumePart)
        {
            isExhausted = false;
            movement->SetRunAllowed(true);
        }
    }

    void StaminaComponent::UpdateRegen(float deltaTime)
    {
        if (stamina >= maxStamina)
        {
            return;
        }

        if (regenTimer > 0.f)
        {
            regenTimer -= deltaTime;
            return;
        }

        stamina = std::min(stamina + regenPerSecond * deltaTime, maxStamina);
        Publish();
    }

    void StaminaComponent::Render()
    {
    }

    void StaminaComponent::Publish()
    {
        changedEvent.Invoke(stamina, maxStamina);
    }

    XYZEngine::SubscriptionId StaminaComponent::SubscribeChanged(std::function<void(float, float)> onChanged)
    {
        return changedEvent.Subscribe(std::move(onChanged));
    }

    void StaminaComponent::UnsubscribeChanged(XYZEngine::SubscriptionId subscription)
    {
        changedEvent.Unsubscribe(subscription);
    }
}
