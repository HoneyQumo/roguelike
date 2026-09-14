#pragma once

#include <string>
#include <vector>
#include <EventList.h>
#include <Vector.h>
#include "DoorHinge.h"
#include "InteractableComponent.h"

namespace XYZEngine
{
    class ColliderComponent;
    struct Collision;
    class TransformComponent;
}

namespace RoguelikeGame
{
    class PropVisualComponent;

    class DoorComponent : public InteractableComponent
    {
    public:
        DoorComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetDoorId(const std::string& newDoorId);
        const std::string& GetDoorId() const;

        void SetKeyName(const std::string& newKeyName);
        void SetVisual(PropVisualComponent* newVisual);
        void SetLeaf(XYZEngine::TransformComponent* newLeaf);
        void SetHinge(const DoorHinge& newHinge);
        void SetReach(XYZEngine::ColliderComponent* reach);

        std::string GetPrompt(XYZEngine::GameObject* actor) const override;
        std::string GetRefusal(XYZEngine::GameObject* actor) const override;
        bool IsAvailable() const override;
        bool Interact(XYZEngine::GameObject* actor) override;

        bool IsOpen() const;
        bool IsSwinging() const;
        float GetLeafAngle() const;

        bool HasKey(XYZEngine::GameObject* actor) const;
        bool TryOpenFor(XYZEngine::GameObject* actor);
        void Open();

        XYZEngine::SubscriptionId SubscribeOpened(std::function<void(const XYZEngine::Vector2Df&)> onOpened);
        XYZEngine::SubscriptionId SubscribeRefused(std::function<void(const XYZEngine::Vector2Df&)> onRefused);

    private:
        void OnCollision(const XYZEngine::Collision& collision);
        void TurnLeaf();

        XYZEngine::ColliderComponent* collider = nullptr;
        PropVisualComponent* visual = nullptr;
        XYZEngine::TransformComponent* leaf = nullptr;

        DoorHinge hinge;
        std::string doorId;
        std::string keyName;
        float swingTime = 0.f;
        bool isOpen = false;

        XYZEngine::EventList<const XYZEngine::Vector2Df&> openedEvent;
        XYZEngine::EventList<const XYZEngine::Vector2Df&> refusedEvent;
    };

    void LinkDoors(const std::vector<DoorComponent*>& doors);
}
