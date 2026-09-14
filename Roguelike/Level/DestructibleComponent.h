#pragma once

#include <functional>
#include <Component.h>
#include <EventList.h>
#include <Vector.h>

namespace XYZEngine
{
    class ColliderComponent;
    class GameObject;
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

        bool IsBroken() const;

        XYZEngine::SubscriptionId SubscribeBroken(std::function<void(const XYZEngine::Vector2Df&)> onBroken);

    private:
        HealthComponent* health = nullptr;
        XYZEngine::ColliderComponent* collider = nullptr;

        bool isBroken = false;

        XYZEngine::EventList<const XYZEngine::Vector2Df&> brokenEvent;

        void Break();
    };
}
