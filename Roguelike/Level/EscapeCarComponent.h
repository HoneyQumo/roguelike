#pragma once

#include <functional>
#include <string>
#include <EventList.h>
#include <Vector.h>
#include <InputSystem.h>
#include "InteractableComponent.h"

namespace RoguelikeGame
{
    class EscapeCarComponent : public InteractableComponent
    {
    public:
        EscapeCarComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetCarId(const std::string& newCarId);
        const std::string& GetCarId() const;
        void SetReachCollider(XYZEngine::ColliderComponent* reach);

        // Место, куда машина встанет: сама она до приезда стоит за краем экрана.
        void SetParkPlace(const XYZEngine::Vector2Df& place);
        const XYZEngine::Vector2Df& GetParkPlace() const;

        void SetHero(XYZEngine::GameObject* newHero);
        void SetArrived(bool newHasArrived);
        bool HasArrived() const;

        bool IsBoarded() const;
        void SetReady(bool newIsReady);
        bool IsReady() const;

        std::string GetPrompt(XYZEngine::GameObject* actor) const override;
        bool IsAvailable() const override;
        bool Interact(XYZEngine::GameObject* actor) override;
        XYZEngine::InputAction GetAction() const override;

        XYZEngine::SubscriptionId SubscribeBoarded(std::function<void()> onBoarded);

        // Герой добежал до конца - пора играть приезд.
        XYZEngine::SubscriptionId SubscribeCalled(std::function<void()> onCalled);

    private:
        std::string carId;
        bool isBoarded = false;
        bool isReady = true;
        bool hasArrived = true;
        bool wasCalled = false;

        XYZEngine::GameObject* hero = nullptr;
        XYZEngine::Vector2Df parkPlace = {0.f, 0.f};

        XYZEngine::EventList<> boardedEvent;
        XYZEngine::EventList<> calledEvent;
    };
}
