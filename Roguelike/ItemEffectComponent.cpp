#include "ItemEffectComponent.h"
#include "InventoryComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    ItemEffectComponent::ItemEffectComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void ItemEffectComponent::Start()
    {
        inventory = gameObject->GetComponent<InventoryComponent>();
        if (inventory == nullptr)
        {
            LOG_ERROR("Item effects need an inventory on " + gameObject->GetName());
            gameObject->DestroyComponent(this);
            return;
        }

        inventory->SetUseHandler([this](const ItemDefinition& item) { return Apply(item); });
    }

    void ItemEffectComponent::Update(float deltaTime)
    {
    }

    void ItemEffectComponent::Render()
    {
    }

    void ItemEffectComponent::SetHandler(ItemEffectKind kind, ItemEffectHandler handler)
    {
        if (kind == ItemEffectKind::None)
        {
            return;
        }

        for (Entry& entry : handlers)
        {
            if (entry.kind == kind)
            {
                entry.handler = std::move(handler);
                return;
            }
        }

        handlers.push_back({kind, std::move(handler)});
    }

    bool ItemEffectComponent::HasHandler(ItemEffectKind kind) const
    {
        return FindHandler(kind) != nullptr;
    }

    const ItemEffectHandler* ItemEffectComponent::FindHandler(ItemEffectKind kind) const
    {
        for (const Entry& entry : handlers)
        {
            if (entry.kind == kind && entry.handler != nullptr)
            {
                return &entry.handler;
            }
        }

        return nullptr;
    }

    bool ItemEffectComponent::Apply(const ItemDefinition& item)
    {
        const ItemEffectHandler* handler = FindHandler(item.effect.kind);
        if (handler == nullptr)
        {
            LOG_INFO("Nothing applies " + item.id + " right now");
            refusedEvent.Invoke(item);
            return false;
        }

        if (!(*handler)(item.effect))
        {
            refusedEvent.Invoke(item);
            return false;
        }

        appliedEvent.Invoke(item);
        return true;
    }

    XYZEngine::SubscriptionId ItemEffectComponent::SubscribeApplied(std::function<void(const ItemDefinition&)> onApplied)
    {
        return appliedEvent.Subscribe(std::move(onApplied));
    }

    XYZEngine::SubscriptionId ItemEffectComponent::SubscribeRefused(std::function<void(const ItemDefinition&)> onRefused)
    {
        return refusedEvent.Subscribe(std::move(onRefused));
    }
}
