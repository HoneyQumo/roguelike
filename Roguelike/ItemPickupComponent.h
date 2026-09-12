#pragma once

#include <functional>
#include <Component.h>
#include <EventList.h>
#include "ItemDefinition.h"

namespace XYZEngine
{
    class ColliderComponent;
    class SpriteRendererComponent;
    class Trigger;
}

namespace RoguelikeGame
{
    class ItemPickupComponent : public XYZEngine::Component
    {
    public:
        ItemPickupComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetDefinition(const ItemDefinition* newDefinition);
        const ItemDefinition* GetDefinition() const;

        bool IsPickedUp() const;
        bool TryPickUp(XYZEngine::GameObject* collector);

        XYZEngine::SubscriptionId SubscribePickedUp(std::function<void(const ItemDefinition&, XYZEngine::GameObject*)> onPickedUp);

    private:
        const ItemDefinition* definition = nullptr;
        XYZEngine::ColliderComponent* collider = nullptr;
        XYZEngine::SpriteRendererComponent* renderer = nullptr;
        bool isPickedUp = false;

        XYZEngine::EventList<const ItemDefinition&, XYZEngine::GameObject*> pickedUpEvent;

        void OnTrigger(const XYZEngine::Trigger& trigger);
        void Hide();
    };
}
