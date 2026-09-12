#include "ItemPickupComponent.h"
#include "FactionComponent.h"
#include "InteractionComponent.h"
#include "InventoryComponent.h"
#include <ColliderComponent.h>
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <SpriteRendererComponent.h>
#include <Trigger.h>

namespace RoguelikeGame
{
    ItemPickupComponent::ItemPickupComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void ItemPickupComponent::Start()
    {
        collider = gameObject->GetComponent<XYZEngine::ColliderComponent>();
        renderer = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();

        if (collider == nullptr)
        {
            LOG_ERROR("Item pickup needs a collider on " + gameObject->GetName());
            gameObject->DestroyComponent(this);
            return;
        }

        collider->SubscribeTriggerEnter([this](const XYZEngine::Trigger& trigger) { OnTriggerEnter(trigger); });
        collider->SubscribeTriggerExit([this](const XYZEngine::Trigger& trigger) { OnTriggerExit(trigger); });
    }

    void ItemPickupComponent::Update(float deltaTime)
    {
    }

    void ItemPickupComponent::Render()
    {
    }

    void ItemPickupComponent::SetDefinition(const ItemDefinition* newDefinition)
    {
        definition = newDefinition;
    }

    const ItemDefinition* ItemPickupComponent::GetDefinition() const
    {
        return definition;
    }

    bool ItemPickupComponent::IsPickedUp() const
    {
        return isPickedUp;
    }

    XYZEngine::GameObject* ItemPickupComponent::GetPlayerOf(const XYZEngine::Trigger& trigger, XYZEngine::ColliderComponent* self)
    {
        XYZEngine::ColliderComponent* other = trigger.GetFirst() == self ? trigger.GetSecond() : trigger.GetFirst();
        if (other == nullptr)
        {
            return nullptr;
        }

        XYZEngine::GameObject* candidate = other->GetGameObject();
        return GetFactionOf(candidate) == Faction::Player ? candidate : nullptr;
    }

    void ItemPickupComponent::OnTriggerEnter(const XYZEngine::Trigger& trigger)
    {
        if (isPickedUp)
        {
            return;
        }

        XYZEngine::GameObject* player = GetPlayerOf(trigger, collider);
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

        TryPickUp(player);
    }

    void ItemPickupComponent::OnTriggerExit(const XYZEngine::Trigger& trigger)
    {
        XYZEngine::GameObject* player = GetPlayerOf(trigger, collider);
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

    bool ItemPickupComponent::TryPickUp(XYZEngine::GameObject* collector)
    {
        if (isPickedUp || definition == nullptr || collector == nullptr)
        {
            return false;
        }

        auto inventory = collector->GetComponent<InventoryComponent>();
        if (inventory != nullptr && !inventory->TryAdd(*definition))
        {
            return false;
        }

        isPickedUp = true;
        Hide();

        LOG_INFO(collector->GetName() + " picks up " + definition->id);

        pickedUpEvent.Invoke(*definition, collector);
        return true;
    }

    void ItemPickupComponent::Hide()
    {
        if (renderer != nullptr)
        {
            renderer->SetEnabled(false);
        }

        if (collider != nullptr)
        {
            gameObject->DestroyComponent(collider);
            collider = nullptr;
        }
    }

    XYZEngine::SubscriptionId ItemPickupComponent::SubscribePickedUp(
        std::function<void(const ItemDefinition&, XYZEngine::GameObject*)> onPickedUp)
    {
        return pickedUpEvent.Subscribe(std::move(onPickedUp));
    }
}
