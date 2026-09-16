#pragma once

#include <functional>
#include <string>
#include <EventList.h>
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

        bool IsBoarded() const;
        void SetReady(bool newIsReady);
        bool IsReady() const;

        std::string GetPrompt(XYZEngine::GameObject* actor) const override;
        bool IsAvailable() const override;
        bool Interact(XYZEngine::GameObject* actor) override;
        XYZEngine::InputAction GetAction() const override;

        XYZEngine::SubscriptionId SubscribeBoarded(std::function<void()> onBoarded);

    private:
        std::string carId;
        bool isBoarded = false;
        bool isReady = true;

        XYZEngine::EventList<> boardedEvent;
    };
}
