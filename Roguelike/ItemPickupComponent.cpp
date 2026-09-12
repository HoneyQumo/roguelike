#include "ItemPickupComponent.h"
#include "FactionComponent.h"
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

        collider->SubscribeTriggerEnter([this](const XYZEngine::Trigger& trigger) { OnTrigger(trigger); });
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

    void ItemPickupComponent::OnTrigger(const XYZEngine::Trigger& trigger)
    {
        if (isPickedUp)
        {
            return;
        }

        XYZEngine::ColliderComponent* other = trigger.GetFirst() == collider ? trigger.GetSecond() : trigger.GetFirst();
        if (other == nullptr)
        {
            return;
        }

        XYZEngine::GameObject* collector = other->GetGameObject();
        if (GetFactionOf(collector) != Faction::Player)
        {
            return;
        }

        TryPickUp(collector);
    }

    bool ItemPickupComponent::TryPickUp(XYZEngine::GameObject* collector)
    {
        if (isPickedUp || definition == nullptr || collector == nullptr)
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
