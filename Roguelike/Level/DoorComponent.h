#pragma once

#include <string>
#include <Component.h>
#include <EventList.h>
#include <Vector.h>
#include "ItemCatalog.h"

namespace XYZEngine
{
    class ColliderComponent;
    class Collision;
}

namespace RoguelikeGame
{
    class PropVisualComponent;

    class DoorComponent : public XYZEngine::Component
    {
    public:
        DoorComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetDoorId(const std::string& newDoorId);
        const std::string& GetDoorId() const;

        void SetVisual(PropVisualComponent* newVisual);

        bool IsOpen() const;
        bool TryOpenFor(XYZEngine::GameObject* actor);

        XYZEngine::SubscriptionId SubscribeOpened(std::function<void(const XYZEngine::Vector2Df&)> onOpened);
        XYZEngine::SubscriptionId SubscribeRefused(std::function<void(const XYZEngine::Vector2Df&)> onRefused);

    private:
        void OnCollision(const XYZEngine::Collision& collision);

        XYZEngine::ColliderComponent* collider = nullptr;
        PropVisualComponent* visual = nullptr;

        std::string doorId;
        bool isOpen = false;

        XYZEngine::EventList<const XYZEngine::Vector2Df&> openedEvent;
        XYZEngine::EventList<const XYZEngine::Vector2Df&> refusedEvent;
    };
}
