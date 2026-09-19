#include "InteractableComponent.h"
#include "FactionComponent.h"
#include "InteractionComponent.h"
#include <ColliderComponent.h>
#include <GameObject.h>
#include <Trigger.h>

namespace RoguelikeGame
{
    XYZEngine::InputAction InteractableComponent::GetAction() const
    {
        return XYZEngine::InputAction::Interact;
    }

    InteractableComponent::InteractableComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    std::string InteractableComponent::GetRefusal(XYZEngine::GameObject* actor) const
    {
        return {};
    }

    float InteractableComponent::GetHoldTime() const
    {
        return 0.f;
    }

    void InteractableComponent::OnHold(float part, float deltaTime)
    {
    }

    void InteractableComponent::OnHoldBroken()
    {
    }

    void InteractableComponent::BindReach(XYZEngine::ColliderComponent* reach)
    {
        if (reach == nullptr)
        {
            return;
        }

        reachCollider = reach;
        reachCollider->SubscribeTriggerEnter([this](const XYZEngine::Trigger& trigger) { OnReachEnter(trigger); });
        reachCollider->SubscribeTriggerExit([this](const XYZEngine::Trigger& trigger) { OnReachExit(trigger); });
    }

    XYZEngine::GameObject* InteractableComponent::GetPlayerOf(const XYZEngine::Trigger& trigger) const
    {
        XYZEngine::ColliderComponent* other = trigger.GetFirst() == reachCollider ? trigger.GetSecond() : trigger.GetFirst();
        if (other == nullptr)
        {
            return nullptr;
        }

        XYZEngine::GameObject* candidate = other->GetGameObject();
        return GetFactionOf(candidate) == Faction::Player ? candidate : nullptr;
    }

    void InteractableComponent::OnReachEnter(const XYZEngine::Trigger& trigger)
    {
        if (!IsAvailable())
        {
            return;
        }

        XYZEngine::GameObject* player = GetPlayerOf(trigger);
        if (player == nullptr)
        {
            return;
        }

        auto interaction = player->GetComponent<InteractionComponent>();
        if (interaction != nullptr)
        {
            interaction->AddCandidate(this);
            return;
        }

        Interact(player);
    }

    void InteractableComponent::OnReachExit(const XYZEngine::Trigger& trigger)
    {
        XYZEngine::GameObject* player = GetPlayerOf(trigger);
        if (player == nullptr)
        {
            return;
        }

        auto interaction = player->GetComponent<InteractionComponent>();
        if (interaction != nullptr)
        {
            interaction->RemoveCandidate(this);
        }
    }
}
