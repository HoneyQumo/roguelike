#pragma once

#include <functional>
#include <Component.h>
#include <Vector.h>
#include "InventoryComponent.h"
#include "ItemDefinition.h"

namespace RoguelikeGame
{
    /**
    *	Компонент знает правило сброса, но не знает, куда именно ложится предмет:
    *	пол принадлежит уровню, а уровень живёт в сцене. Поэтому место рождения
    *	внедряется снаружи - и правило проверяется без движка.
    */
    class ItemDropComponent : public XYZEngine::Component
    {
    public:
        using Spawner = std::function<bool(const ItemDefinition& item, int count, int charge,
            const XYZEngine::Vector2Df& position)>;

        ItemDropComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetInventory(InventoryComponent* newInventory);
        void SetSpawner(Spawner newSpawner);

        bool Drop(int slotIndex);

    private:
        InventoryComponent* inventory = nullptr;
        Spawner spawner;

        XYZEngine::Vector2Df DropPlace() const;
    };
}
