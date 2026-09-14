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
    class InteractableComponent;

    class InteractionComponent : public XYZEngine::Component
    {
    public:
        InteractionComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void AddCandidate(InteractableComponent* candidate);
        void RemoveCandidate(InteractableComponent* candidate);

        InteractableComponent* GetTarget() const;
        std::string GetPrompt() const;
        bool Interact();

        XYZEngine::SubscriptionId SubscribePromptChanged(std::function<void(const std::string&)> onPromptChanged);
        XYZEngine::SubscriptionId SubscribeRefused(std::function<void(const std::string&)> onRefused);

    private:
        XYZEngine::InputComponent* input = nullptr;
        XYZEngine::TransformComponent* transform = nullptr;

        std::vector<InteractableComponent*> candidates;
        InteractableComponent* target = nullptr;
        std::string prompt;

        XYZEngine::EventList<const std::string&> promptChangedEvent;
        XYZEngine::EventList<const std::string&> refusedEvent;

        void UpdateTarget();
        InteractableComponent* FindNearest() const;
        void SetTarget(InteractableComponent* newTarget);
    };
}
