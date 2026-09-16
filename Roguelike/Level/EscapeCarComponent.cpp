#include "EscapeCarComponent.h"
#include "GameSettings.h"
#include <ColliderComponent.h>
#include <GameObject.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    EscapeCarComponent::EscapeCarComponent(XYZEngine::GameObject* gameObject) : InteractableComponent(gameObject) {}

    void EscapeCarComponent::Update(float deltaTime)
    {
    }

    void EscapeCarComponent::Render()
    {
    }

    void EscapeCarComponent::SetCarId(const std::string& newCarId)
    {
        carId = newCarId;
    }

    const std::string& EscapeCarComponent::GetCarId() const
    {
        return carId;
    }

    void EscapeCarComponent::SetReachCollider(XYZEngine::ColliderComponent* reach)
    {
        BindReach(reach);
    }

    bool EscapeCarComponent::IsBoarded() const
    {
        return isBoarded;
    }

    std::string EscapeCarComponent::GetPrompt(XYZEngine::GameObject* actor) const
    {
        return ESCAPE_CAR_PROMPT;
    }

    void EscapeCarComponent::SetReady(bool newIsReady)
    {
        isReady = newIsReady;
    }

    bool EscapeCarComponent::IsReady() const
    {
        return isReady;
    }

    bool EscapeCarComponent::IsAvailable() const
    {
        return isReady && !isBoarded;
    }

    bool EscapeCarComponent::Interact(XYZEngine::GameObject* actor)
    {
        if (!IsAvailable())
        {
            return false;
        }

        isBoarded = true;
        LOG_INFO("Player boards the escape car " + carId);
        boardedEvent.Invoke();

        return true;
    }

    XYZEngine::InputAction EscapeCarComponent::GetAction() const
    {
        return XYZEngine::InputAction::Pass;
    }

    XYZEngine::SubscriptionId EscapeCarComponent::SubscribeBoarded(std::function<void()> onBoarded)
    {
        return boardedEvent.Subscribe(std::move(onBoarded));
    }
}
