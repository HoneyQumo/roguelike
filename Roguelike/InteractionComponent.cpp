#include "InteractionComponent.h"
#include "GameSettings.h"
#include "ItemPickupComponent.h"
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

    void InteractionComponent::AddCandidate(ItemPickupComponent* candidate)
    {
        if (candidate == nullptr || std::find(candidates.begin(), candidates.end(), candidate) != candidates.end())
        {
            return;
        }

        candidates.push_back(candidate);
    }

    void InteractionComponent::RemoveCandidate(ItemPickupComponent* candidate)
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
            [](ItemPickupComponent* candidate) { return candidate == nullptr || candidate->IsPickedUp(); }), candidates.end());

        SetTarget(FindNearest());
    }

    ItemPickupComponent* InteractionComponent::FindNearest() const
    {
        ItemPickupComponent* nearest = nullptr;
        float nearestDistance = 0.f;

        for (ItemPickupComponent* candidate : candidates)
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

    void InteractionComponent::SetTarget(ItemPickupComponent* newTarget)
    {
        if (target == newTarget)
        {
            return;
        }

        target = newTarget;
        prompt.clear();

        if (target != nullptr && target->GetDefinition() != nullptr)
        {
            prompt = std::string(INTERACT_PROMPT_PREFIX) + target->GetDefinition()->name;
        }

        promptChangedEvent.Invoke(prompt);
    }

    ItemPickupComponent* InteractionComponent::GetTarget() const
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

        ItemPickupComponent* picked = target;
        if (!picked->TryPickUp(gameObject))
        {
            return false;
        }

        RemoveCandidate(picked);
        UpdateTarget();

        return true;
    }

    XYZEngine::SubscriptionId InteractionComponent::SubscribePromptChanged(std::function<void(const std::string&)> onPromptChanged)
    {
        return promptChangedEvent.Subscribe(std::move(onPromptChanged));
    }
}
