#pragma once

#include <functional>
#include <Component.h>
#include <EventList.h>
#include <Vector.h>
#include <SFML/Graphics/Color.hpp>

namespace XYZEngine
{
    class ColliderComponent;
    class GameObject;
    class RectangleRendererComponent;
}

namespace RoguelikeGame
{
    class HealthComponent;

    class DestructibleComponent : public XYZEngine::Component
    {
    public:
        DestructibleComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetBrokenColor(const sf::Color& newBrokenColor);

        bool IsBroken() const;

        XYZEngine::SubscriptionId SubscribeBroken(std::function<void(const XYZEngine::Vector2Df&)> onBroken);

    private:
        HealthComponent* health = nullptr;
        XYZEngine::ColliderComponent* collider = nullptr;
        XYZEngine::RectangleRendererComponent* renderer = nullptr;

        sf::Color brokenColor = {80, 60, 35};
        bool isBroken = false;

        XYZEngine::EventList<const XYZEngine::Vector2Df&> brokenEvent;

        void Break();
    };
}
