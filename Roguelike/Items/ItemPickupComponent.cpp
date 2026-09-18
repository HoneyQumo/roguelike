#include "ItemPickupComponent.h"
#include "GameSettings.h"
#include "InventoryComponent.h"
#include "ItemEffectComponent.h"
#include <ColliderComponent.h>
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <SpriteRendererComponent.h>

namespace RoguelikeGame
{
    ItemPickupComponent::ItemPickupComponent(XYZEngine::GameObject* gameObject) : InteractableComponent(gameObject) {}

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

        BindReach(collider);
    }

    void ItemPickupComponent::Update(float deltaTime)
    {
    }

    void ItemPickupComponent::Render()
    {
    }

    void ItemPickupComponent::SetDefinition(const ItemDefinition* newDefinition)
    {
        hasDefinition = newDefinition != nullptr;
        definition = hasDefinition ? *newDefinition : ItemDefinition();
    }

    const ItemDefinition* ItemPickupComponent::GetDefinition() const
    {
        return hasDefinition ? &definition : nullptr;
    }

    bool ItemPickupComponent::IsPickedUp() const
    {
        return isPickedUp;
    }

    std::string ItemPickupComponent::GetPrompt(XYZEngine::GameObject* actor) const
    {
        if (!hasDefinition)
        {
            return {};
        }

        return std::string(INTERACT_PROMPT_PREFIX) + definition.name;
    }

    bool ItemPickupComponent::IsAvailable() const
    {
        return !isPickedUp;
    }

    bool ItemPickupComponent::Interact(XYZEngine::GameObject* actor)
    {
        return TryPickUp(actor);
    }

    bool ItemPickupComponent::TryPickUp(XYZEngine::GameObject* collector)
    {
        if (isPickedUp || !hasDefinition || collector == nullptr)
        {
            return false;
        }

        if (!Take(definition, collector))
        {
            return false;
        }

        isPickedUp = true;
        Hide();

        LOG_INFO(collector->GetName() + " picks up " + definition.id);

        pickedUpEvent.Invoke(definition, collector);
        return true;
    }

    bool ItemPickupComponent::Take(const ItemDefinition& item, XYZEngine::GameObject* collector)
    {
        auto effects = collector->GetComponent<ItemEffectComponent>();
        if (effects != nullptr && effects->AppliesOnPickup(item))
        {
            return effects->Apply(item).isApplied;
        }

        auto inventory = collector->GetComponent<InventoryComponent>();

        return inventory == nullptr || inventory->TryAdd(item);
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
