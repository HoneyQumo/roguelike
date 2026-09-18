#include "InventoryComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <algorithm>

namespace RoguelikeGame
{
    bool InventorySlot::IsEmpty() const
    {
        return count <= 0;
    }

    bool InventorySlot::Holds(const std::string& itemId) const
    {
        return !IsEmpty() && item.id == itemId;
    }

    int InventorySlot::FreeSpace() const
    {
        if (IsEmpty())
        {
            return 0;
        }

        int maxStack = item.stackable ? std::max(item.maxStack, 1) : 1;
        return maxStack - count;
    }

    InventoryComponent::InventoryComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        slots.resize(INVENTORY_CAPACITY);
    }

    void InventoryComponent::Update(float deltaTime)
    {
    }

    void InventoryComponent::Render()
    {
    }

    void InventoryComponent::SetCapacity(int newCapacity)
    {
        if (newCapacity <= 0)
        {
            LOG_WARN("Inventory capacity must be positive on " + gameObject->GetName());
            return;
        }

        slots.assign(newCapacity, InventorySlot());
        selectedSlot = 0;

        changedEvent.Invoke();
    }

    int InventoryComponent::GetCapacity() const
    {
        return static_cast<int>(slots.size());
    }

    int InventoryComponent::MaxStackOf(const ItemDefinition& item) const
    {
        return item.stackable ? std::max(item.maxStack, 1) : 1;
    }

    int InventoryComponent::FindStackWithSpace(const ItemDefinition& item) const
    {
        if (!item.stackable)
        {
            return -1;
        }

        for (int index = 0; index < GetCapacity(); index++)
        {
            // В заряженной ячейке лежит конкретный ствол со своими патронами - он не стопка.
            if (slots[index].Holds(item.id) && slots[index].charge == NO_CHARGE && slots[index].FreeSpace() > 0)
            {
                return index;
            }
        }

        return -1;
    }

    int InventoryComponent::CountFreeSpaceFor(const ItemDefinition& item) const
    {
        int maxStack = MaxStackOf(item);
        int space = 0;

        for (const InventorySlot& slot : slots)
        {
            if (slot.IsEmpty())
            {
                space += maxStack;
                continue;
            }

            if (item.stackable && slot.Holds(item.id))
            {
                space += slot.FreeSpace();
            }
        }

        return space;
    }

    int InventoryComponent::FindEmptySlot() const
    {
        for (int index = 0; index < GetCapacity(); index++)
        {
            if (slots[index].IsEmpty())
            {
                return index;
            }
        }

        return -1;
    }

    bool InventoryComponent::IsValidSlot(int index) const
    {
        return index >= 0 && index < GetCapacity();
    }

    bool InventoryComponent::TryAdd(const ItemDefinition& item, int count, int charge)
    {
        if (count <= 0)
        {
            return false;
        }

        int maxStack = MaxStackOf(item);

        if (CountFreeSpaceFor(item) < count)
        {
            LOG_WARN("Inventory is full, " + item.id + " is left on the ground");
            rejectedEvent.Invoke(item);
            return false;
        }

        int left = count;
        while (left > 0)
        {
            int slotIndex = FindStackWithSpace(item);
            if (slotIndex < 0)
            {
                slotIndex = FindEmptySlot();
            }

            InventorySlot& slot = slots[slotIndex];
            if (slot.IsEmpty())
            {
                slot.item = item;
                slot.count = 0;

                // Заряд есть только у ствола, а ствол не стопкуется: в стопке ему негде лежать.
                slot.charge = item.stackable ? NO_CHARGE : charge;
            }

            int taken = std::min(left, maxStack - slot.count);
            slot.count += taken;
            left -= taken;
        }

        addedEvent.Invoke(item, count);
        changedEvent.Invoke();

        return true;
    }

    bool InventoryComponent::Remove(int slotIndex, int count)
    {
        if (!IsValidSlot(slotIndex) || count <= 0)
        {
            return false;
        }

        InventorySlot& slot = slots[slotIndex];
        if (slot.IsEmpty() || slot.count < count)
        {
            return false;
        }

        ItemDefinition removed = slot.item;
        slot.count -= count;

        if (slot.count <= 0)
        {
            slot.item = ItemDefinition();
            slot.count = 0;
            slot.charge = NO_CHARGE;
        }

        removedEvent.Invoke(removed, count);
        changedEvent.Invoke();

        return true;
    }

    bool InventoryComponent::Replace(int slotIndex, const ItemDefinition& item, int charge)
    {
        if (!IsValidSlot(slotIndex))
        {
            return false;
        }

        InventorySlot& slot = slots[slotIndex];
        bool wasTaken = !slot.IsEmpty();
        ItemDefinition removed = slot.item;
        int removedCount = slot.count;

        slot.item = item;
        slot.count = 1;
        slot.charge = charge;

        if (wasTaken)
        {
            removedEvent.Invoke(removed, removedCount);
        }

        addedEvent.Invoke(item, 1);
        changedEvent.Invoke();

        return true;
    }

    bool InventoryComponent::Use(int slotIndex)
    {
        if (!IsValidSlot(slotIndex))
        {
            return false;
        }

        InventorySlot& slot = slots[slotIndex];
        if (slot.IsEmpty())
        {
            return false;
        }

        ItemDefinition used = slot.item;

        // Сколько тратить, решает эффект: сумка не знает, что бывают предметы на два глотка.
        ItemUseResult result = useHandler != nullptr ? useHandler(used) : ItemUseResult(true);
        if (!result.isApplied || result.consumed <= 0)
        {
            return false;
        }

        usedEvent.Invoke(used);

        return Remove(slotIndex, result.consumed);
    }

    bool InventoryComponent::UseSelected()
    {
        return Use(selectedSlot);
    }

    void InventoryComponent::SetUseHandler(std::function<ItemUseResult(const ItemDefinition&)> newUseHandler)
    {
        useHandler = std::move(newUseHandler);
    }

    void InventoryComponent::SelectSlot(int index)
    {
        if (!IsValidSlot(index))
        {
            return;
        }

        selectedSlot = index;
        changedEvent.Invoke();
    }

    int InventoryComponent::GetSelectedSlot() const
    {
        return selectedSlot;
    }

    const InventorySlot& InventoryComponent::GetSlot(int index) const
    {
        static const InventorySlot emptySlot;
        return IsValidSlot(index) ? slots[index] : emptySlot;
    }

    int InventoryComponent::GetUsedSlots() const
    {
        int used = 0;
        for (const InventorySlot& slot : slots)
        {
            if (!slot.IsEmpty())
            {
                used++;
            }
        }

        return used;
    }

    bool InventoryComponent::IsFull() const
    {
        return FindEmptySlot() < 0;
    }

    bool InventoryComponent::Contains(const std::string& itemId) const
    {
        return CountOf(itemId) > 0;
    }

    int InventoryComponent::CountOf(const std::string& itemId) const
    {
        int total = 0;
        for (const InventorySlot& slot : slots)
        {
            if (slot.Holds(itemId))
            {
                total += slot.count;
            }
        }

        return total;
    }

    int InventoryComponent::FindSlot(const std::string& itemId) const
    {
        for (int index = 0; index < slots.size(); index++)
        {
            if (slots[index].Holds(itemId))
            {
                return index;
            }
        }

        return -1;
    }

    bool InventoryComponent::RemoveById(const std::string& itemId, int count)
    {
        int index = FindSlot(itemId);

        return index >= 0 && Remove(index, count);
    }

    XYZEngine::SubscriptionId InventoryComponent::SubscribeAdded(std::function<void(const ItemDefinition&, int)> onAdded)
    {
        return addedEvent.Subscribe(std::move(onAdded));
    }

    XYZEngine::SubscriptionId InventoryComponent::SubscribeRemoved(std::function<void(const ItemDefinition&, int)> onRemoved)
    {
        return removedEvent.Subscribe(std::move(onRemoved));
    }

    XYZEngine::SubscriptionId InventoryComponent::SubscribeUsed(std::function<void(const ItemDefinition&)> onUsed)
    {
        return usedEvent.Subscribe(std::move(onUsed));
    }

    XYZEngine::SubscriptionId InventoryComponent::SubscribeRejected(std::function<void(const ItemDefinition&)> onRejected)
    {
        return rejectedEvent.Subscribe(std::move(onRejected));
    }

    XYZEngine::SubscriptionId InventoryComponent::SubscribeChanged(std::function<void()> onChanged)
    {
        return changedEvent.Subscribe(std::move(onChanged));
    }
}
