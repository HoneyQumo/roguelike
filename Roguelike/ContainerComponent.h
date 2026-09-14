#pragma once

#include <functional>
#include <string>
#include <SFML/Graphics/Color.hpp>
#include <EventList.h>
#include <Vector.h>
#include "InteractableComponent.h"

namespace XYZEngine
{
    class RectangleRendererComponent;
}

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
        void SetOpenedColor(const sf::Color& newOpenedColor);
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
        sf::Color openedColor = {90, 80, 55};
        bool isOpen = false;

        XYZEngine::RectangleRendererComponent* renderer = nullptr;

        XYZEngine::EventList<const XYZEngine::Vector2Df&> openedEvent;
    };
}
