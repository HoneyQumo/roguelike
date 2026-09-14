#pragma once

#include <functional>
#include <Component.h>
#include <EventList.h>

namespace XYZEngine
{
    class MovementComponent;
}

namespace RoguelikeGame
{
    class StaminaComponent : public XYZEngine::Component
    {
    public:
        StaminaComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetMaxStamina(float newMaxStamina);
        void SetRegen(float newRegenPerSecond, float newRegenDelay);
        void SetRunDrain(float newRunDrainPerSecond);
        void SetRunResumePart(float newRunResumePart);

        float GetMaxStamina() const;
        float GetStamina() const;
        float GetStaminaPercent() const;
        bool IsExhausted() const;

        bool HasStamina(float amount) const;
        bool TrySpend(float amount);

        XYZEngine::SubscriptionId SubscribeChanged(std::function<void(float, float)> onChanged);
        void UnsubscribeChanged(XYZEngine::SubscriptionId subscription);

    private:
        XYZEngine::MovementComponent* movement = nullptr;

        float maxStamina = 100.f;
        float stamina = 100.f;
        float regenPerSecond = 18.f;
        float regenDelay = 0.8f;
        float runDrainPerSecond = 22.f;
        float runResumePart = 0.2f;
        float regenTimer = 0.f;
        bool isExhausted = false;

        XYZEngine::EventList<float, float> changedEvent;

        void Spend(float amount);
        void UpdateRun(float deltaTime);
        void UpdateRegen(float deltaTime);
        void Publish();
    };
}
