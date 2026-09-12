#pragma once

#include <functional>
#include <string>
#include <vector>
#include <Component.h>
#include <EventList.h>
#include "ItemDefinition.h"

namespace RoguelikeGame
{
    struct InventorySlot
    {
        const ItemDefinition* item = nullptr;
        int count = 0;

        bool IsEmpty() const;
        bool Holds(const std::string& itemId) const;
        int FreeSpace() const;
    };

    class InventoryComponent : public XYZEngine::Component
    {
    public:
        InventoryComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetCapacity(int newCapacity);
        int GetCapacity() const;

        bool TryAdd(const ItemDefinition& item, int count = 1);
        bool Remove(int slotIndex, int count = 1);
        bool Use(int slotIndex);
        bool UseSelected();

        void SelectSlot(int index);
        int GetSelectedSlot() const;

        const InventorySlot& GetSlot(int index) const;
        int GetUsedSlots() const;
        bool IsFull() const;

        bool Contains(const std::string& itemId) const;
        int CountOf(const std::string& itemId) const;

        XYZEngine::SubscriptionId SubscribeAdded(std::function<void(const ItemDefinition&, int)> onAdded);
        XYZEngine::SubscriptionId SubscribeRemoved(std::function<void(const ItemDefinition&, int)> onRemoved);
        XYZEngine::SubscriptionId SubscribeUsed(std::function<void(const ItemDefinition&)> onUsed);
        XYZEngine::SubscriptionId SubscribeRejected(std::function<void(const ItemDefinition&)> onRejected);
        XYZEngine::SubscriptionId SubscribeChanged(std::function<void()> onChanged);

    private:
        std::vector<InventorySlot> slots;
        int selectedSlot = 0;

        XYZEngine::EventList<const ItemDefinition&, int> addedEvent;
        XYZEngine::EventList<const ItemDefinition&, int> removedEvent;
        XYZEngine::EventList<const ItemDefinition&> usedEvent;
        XYZEngine::EventList<const ItemDefinition&> rejectedEvent;
        XYZEngine::EventList<> changedEvent;

        int MaxStackOf(const ItemDefinition& item) const;
        int CountFreeSpaceFor(const ItemDefinition& item) const;
        int FindStackWithSpace(const ItemDefinition& item) const;
        int FindEmptySlot() const;
        bool IsValidSlot(int index) const;
    };
}
