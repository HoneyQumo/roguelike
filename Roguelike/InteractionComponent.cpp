#include "InteractionComponent.h"
#include "GameSettings.h"
#include "InteractableComponent.h"
#include <GameObject.h>
#include <InputComponent.h>
#include <LoggerRegistry.h>
#include <TextUtils.h>
#include <TransformComponent.h>
#include <algorithm>

namespace RoguelikeGame
{
    InteractionComponent::InteractionComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
    }

    void InteractionComponent::Start()
    {
        input = gameObject->GetComponent<XYZEngine::InputComponent>();
    }

    void InteractionComponent::AddCandidate(InteractableComponent* candidate)
    {
        if (candidate == nullptr || std::find(candidates.begin(), candidates.end(), candidate) != candidates.end())
        {
            return;
        }

        candidates.push_back(candidate);
    }

    void InteractionComponent::RemoveCandidate(InteractableComponent* candidate)
    {
        candidates.erase(std::remove(candidates.begin(), candidates.end(), candidate), candidates.end());

        if (target == candidate)
        {
            SetTarget(nullptr);
        }
    }

    void InteractionComponent::Update(float deltaTime)
    {
        UpdateTarget();

        if (input != nullptr && input->WasActionPressed(XYZEngine::InputAction::Interact))
        {
            Interact();
        }
    }

    void InteractionComponent::Render()
    {
    }

    void InteractionComponent::UpdateTarget()
    {
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
            [](InteractableComponent* candidate) { return candidate == nullptr || !candidate->IsAvailable(); }), candidates.end());

        SetTarget(FindNearest());
    }

    InteractableComponent* InteractionComponent::FindNearest() const
    {
        InteractableComponent* nearest = nullptr;
        float nearestDistance = 0.f;

        for (InteractableComponent* candidate : candidates)
        {
            float distance = (candidate->GetGameObject()->GetTransform()->GetWorldPosition()
                - transform->GetWorldPosition()).GetLengthSquared();

            if (nearest == nullptr || distance < nearestDistance)
            {
                nearest = candidate;
                nearestDistance = distance;
            }
        }

        return nearest;
    }

    void InteractionComponent::SetTarget(InteractableComponent* newTarget)
    {
        target = newTarget;

        std::string wanted = target != nullptr ? target->GetPrompt(gameObject) : std::string();
        if (wanted == prompt)
        {
            return;
        }

        prompt = std::move(wanted);
        promptChangedEvent.Invoke(prompt);
    }

    InteractableComponent* InteractionComponent::GetTarget() const
    {
        return target;
    }

    std::string InteractionComponent::GetPrompt() const
    {
        return prompt;
    }

    bool InteractionComponent::Interact()
    {
        if (target == nullptr)
        {
            return false;
        }

        InteractableComponent* chosen = target;
        if (!chosen->Interact(gameObject))
        {
            refusedEvent.Invoke(chosen->GetRefusal(gameObject));
            return false;
        }

        UpdateTarget();

        return true;
    }

    XYZEngine::SubscriptionId InteractionComponent::SubscribePromptChanged(std::function<void(const std::string&)> onPromptChanged)
    {
        return promptChangedEvent.Subscribe(std::move(onPromptChanged));
    }

    XYZEngine::SubscriptionId InteractionComponent::SubscribeRefused(std::function<void(const std::string&)> onRefused)
    {
        return refusedEvent.Subscribe(std::move(onRefused));
    }
}
