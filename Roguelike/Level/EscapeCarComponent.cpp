#include "EscapeCarComponent.h"
#include "GameSettings.h"
#include <ColliderComponent.h>
#include <GameObject.h>
#include <TransformComponent.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    EscapeCarComponent::EscapeCarComponent(XYZEngine::GameObject* gameObject) : InteractableComponent(gameObject) {}

    /**
    *	Сторожит подъезд героя к месту, где машина встанет.
    *
    *	Считает расстояние до места, а не до себя: на время приезда машина
    *	едет, и её собственная точка ничего не говорит.
    */
    void EscapeCarComponent::Update(float deltaTime)
    {
        if (hero == nullptr || wasCalled || hasArrived)
        {
            return;
        }

        if ((hero->GetTransform()->GetWorldPosition() - parkPlace).GetLength() > ARRIVAL_CALL_RANGE)
        {
            return;
        }

        wasCalled = true;
        LOG_INFO("Escape car " + carId + " is called");
        calledEvent.Invoke();
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
        return isReady && hasArrived && !isBoarded;
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

    XYZEngine::SubscriptionId EscapeCarComponent::SubscribeCalled(std::function<void()> onCalled)
    {
        return calledEvent.Subscribe(std::move(onCalled));
    }

    void EscapeCarComponent::SetParkPlace(const XYZEngine::Vector2Df& place)
    {
        parkPlace = place;
    }

    const XYZEngine::Vector2Df& EscapeCarComponent::GetParkPlace() const
    {
        return parkPlace;
    }

    void EscapeCarComponent::SetHero(XYZEngine::GameObject* newHero)
    {
        hero = newHero;
    }

    void EscapeCarComponent::SetArrived(bool newHasArrived)
    {
        hasArrived = newHasArrived;
    }

    bool EscapeCarComponent::HasArrived() const
    {
        return hasArrived;
    }
}
