#pragma once

#include <functional>
#include <string>
#include <vector>
#include <Component.h>
#include <EventList.h>
#include "ItemDefinition.h"

namespace RoguelikeGame
{
    // Ствол без заряда ещё не был в руках - такому положен полный магазин.
    constexpr int NO_CHARGE = -1;

    struct InventorySlot
    {
        ItemDefinition item;
        int count = 0;

        // Патроны в магазине лежащего ствола: у остальных предметов заряда нет.
        int charge = NO_CHARGE;

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

        // Кладёт предмет вместо того, что лежало в ячейке: так идёт обмен оружием.
        bool Replace(int slotIndex, const ItemDefinition& item, int charge = NO_CHARGE);
        bool Use(int slotIndex);
        void SetUseHandler(std::function<bool(const ItemDefinition&)> newUseHandler);
        bool UseSelected();

        void SelectSlot(int index);
        int GetSelectedSlot() const;

        const InventorySlot& GetSlot(int index) const;
        int GetUsedSlots() const;
        bool IsFull() const;

        bool Contains(const std::string& itemId) const;
        int CountOf(const std::string& itemId) const;
        int FindSlot(const std::string& itemId) const;
        bool RemoveById(const std::string& itemId, int count = 1);

        XYZEngine::SubscriptionId SubscribeAdded(std::function<void(const ItemDefinition&, int)> onAdded);
        XYZEngine::SubscriptionId SubscribeRemoved(std::function<void(const ItemDefinition&, int)> onRemoved);
        XYZEngine::SubscriptionId SubscribeUsed(std::function<void(const ItemDefinition&)> onUsed);
        XYZEngine::SubscriptionId SubscribeRejected(std::function<void(const ItemDefinition&)> onRejected);
        XYZEngine::SubscriptionId SubscribeChanged(std::function<void()> onChanged);

    private:
        std::vector<InventorySlot> slots;
        int selectedSlot = 0;
        std::function<bool(const ItemDefinition&)> useHandler;

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
