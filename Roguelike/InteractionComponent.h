#pragma once

#include <functional>
#include <string>
#include <vector>
#include <Component.h>
#include <EventList.h>

namespace XYZEngine
{
    class InputComponent;
    class TransformComponent;
}

namespace RoguelikeGame
{
    class ItemPickupComponent;

    class InteractionComponent : public XYZEngine::Component
    {
    public:
        InteractionComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void AddCandidate(ItemPickupComponent* candidate);
        void RemoveCandidate(ItemPickupComponent* candidate);

        ItemPickupComponent* GetTarget() const;
        std::string GetPrompt() const;
        bool Interact();

        XYZEngine::SubscriptionId SubscribePromptChanged(std::function<void(const std::string&)> onPromptChanged);

    private:
        XYZEngine::InputComponent* input = nullptr;
        XYZEngine::TransformComponent* transform = nullptr;

        std::vector<ItemPickupComponent*> candidates;
        ItemPickupComponent* target = nullptr;
        std::string prompt;

        XYZEngine::EventList<const std::string&> promptChangedEvent;

        void UpdateTarget();
        ItemPickupComponent* FindNearest() const;
        void SetTarget(ItemPickupComponent* newTarget);
    };
}
