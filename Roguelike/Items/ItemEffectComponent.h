#pragma once

#include <functional>
#include <vector>
#include <Component.h>
#include <EventList.h>
#include "ItemDefinition.h"

namespace XYZEngine
{
    class GameObject;
}

namespace RoguelikeGame
{
    class InventoryComponent;

    using ItemEffectHandler = std::function<bool(const ItemEffect&)>;
    using ItemPickupRule = std::function<bool(const ItemEffect&)>;

    class ItemEffectComponent : public XYZEngine::Component
    {
    public:
        ItemEffectComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetHandler(ItemEffectKind kind, ItemEffectHandler handler);
        bool HasHandler(ItemEffectKind kind) const;

        void SetPickupRule(ItemEffectKind kind, ItemPickupRule rule);
        bool AppliesOnPickup(const ItemDefinition& item) const;

        bool Apply(const ItemDefinition& item);

        XYZEngine::SubscriptionId SubscribeApplied(std::function<void(const ItemDefinition&)> onApplied);
        XYZEngine::SubscriptionId SubscribeRefused(std::function<void(const ItemDefinition&)> onRefused);

    private:
        struct Entry
        {
            ItemEffectKind kind = ItemEffectKind::None;
            ItemEffectHandler handler;
        };

        struct RuleEntry
        {
            ItemEffectKind kind = ItemEffectKind::None;
            ItemPickupRule rule;
        };

        InventoryComponent* inventory = nullptr;
        std::vector<Entry> handlers;
        std::vector<RuleEntry> pickupRules;

        XYZEngine::EventList<const ItemDefinition&> appliedEvent;
        XYZEngine::EventList<const ItemDefinition&> refusedEvent;

        const ItemEffectHandler* FindHandler(ItemEffectKind kind) const;
    };
}
