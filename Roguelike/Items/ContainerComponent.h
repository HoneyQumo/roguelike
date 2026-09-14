#pragma once

#include <functional>
#include <string>
#include <EventList.h>
#include <Vector.h>
#include "InteractableComponent.h"

namespace RoguelikeGame
{
    class ContainerComponent : public InteractableComponent
    {
    public:
        ContainerComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetTitle(std::string newTitle);
        void SetKeyItem(std::string itemId, std::string keyName);
        void SetReach(XYZEngine::ColliderComponent* reach);

        bool IsLocked() const;
        bool IsOpen() const;
        bool IsOpenableBy(XYZEngine::GameObject* actor) const;

        std::string GetPrompt(XYZEngine::GameObject* actor) const override;
        std::string GetRefusal(XYZEngine::GameObject* actor) const override;
        bool IsAvailable() const override;
        bool Interact(XYZEngine::GameObject* actor) override;

        XYZEngine::SubscriptionId SubscribeOpened(std::function<void(const XYZEngine::Vector2Df&)> onOpened);

    private:
        std::string title;
        std::string keyItem;
        std::string keyName;
        bool isOpen = false;

        XYZEngine::EventList<const XYZEngine::Vector2Df&> openedEvent;
    };
}
