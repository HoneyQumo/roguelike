#pragma once

#include <functional>
#include <string>
#include <vector>
#include <Component.h>
#include <EventList.h>
#include <Vector.h>

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

        bool IsHolding() const;
        float GetHoldPart() const;

        XYZEngine::SubscriptionId SubscribeHoldChanged(std::function<void(float)> onHoldChanged);

        XYZEngine::SubscriptionId SubscribePromptChanged(std::function<void(const std::string&)> onPromptChanged);
        XYZEngine::SubscriptionId SubscribeRefused(std::function<void(const std::string&)> onRefused);

    private:
        XYZEngine::InputComponent* input = nullptr;
        XYZEngine::TransformComponent* transform = nullptr;

        std::vector<InteractableComponent*> candidates;
        InteractableComponent* target = nullptr;
        std::string prompt;

        InteractableComponent* held = nullptr;
        float heldTime = 0.f;
        float heldPart = 0.f;
        XYZEngine::Vector2Df heldFrom;

        XYZEngine::EventList<const std::string&> promptChangedEvent;
        XYZEngine::EventList<const std::string&> refusedEvent;
        XYZEngine::EventList<float> holdChangedEvent;

        void UpdateTarget();
        InteractableComponent* FindNearest() const;
        void SetTarget(InteractableComponent* newTarget);

        void StartHold(InteractableComponent* wanted);
        void KeepHolding(float deltaTime);
        void BreakHold();
        void SetHoldPart(float part);
    };
}
