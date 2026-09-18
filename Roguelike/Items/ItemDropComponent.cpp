#include "ItemDropComponent.h"
#include "GameSettings.h"
#include "InventoryAccess.h"
#include <GameObject.h>

namespace RoguelikeGame
{
    ItemDropComponent::ItemDropComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void ItemDropComponent::Update(float deltaTime)
    {
    }

    void ItemDropComponent::Render()
    {
    }

    void ItemDropComponent::SetInventory(InventoryComponent* newInventory)
    {
        inventory = newInventory;
    }

    void ItemDropComponent::SetSpawner(Spawner newSpawner)
    {
        spawner = std::move(newSpawner);
    }

    bool ItemDropComponent::Drop(int slotIndex)
    {
        if (!CanUseInventory(inventory) || !spawner)
        {
            return false;
        }

        if (slotIndex < 0 || slotIndex >= inventory->GetCapacity())
        {
            return false;
        }

        const InventorySlot& carried = inventory->GetSlot(slotIndex);
        if (carried.IsEmpty())
        {
            return false;
        }

        // Копия: после Remove ячейка уже чужая, а спавнеру нужны id, счёт и магазин.
        ItemDefinition item = carried.item;
        int count = carried.count;
        int charge = carried.charge;

        // Сначала пол, потом сумка: если предмету негде родиться, он не должен исчезнуть.
        if (!spawner(item, count, charge, DropPlace()))
        {
            return false;
        }

        return inventory->Remove(slotIndex, count);
    }

    XYZEngine::Vector2Df ItemDropComponent::DropPlace() const
    {
        return gameObject->GetTransform()->GetWorldPosition() - XYZEngine::Vector2Df{0.f, DROP_STEP};
    }
}
