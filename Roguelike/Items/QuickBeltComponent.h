#pragma once

#include <string>
#include <vector>
#include <Component.h>
#include "InventoryComponent.h"
#include "ItemEffectComponent.h"

namespace RoguelikeGame
{
    /**
    *	Пояс - ярлыки на сумку, а не второе хранилище.
    *
    *	Крючок помнит только id предмета: сколько его осталось, всегда спрашивается
    *	у сумки. Поэтому разъехаться им негде, и после подбора или траты счётчик
    *	верен сам собой.
    */
    class QuickBeltComponent : public XYZEngine::Component
    {
    public:
        QuickBeltComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override {}

        void SetInventory(InventoryComponent* newInventory);
        void SetEffects(ItemEffectComponent* newEffects);

        bool Bind(int hook, const std::string& itemId);
        const std::string& GetBinding(int hook) const;
        int GetCountOn(int hook) const;
        int GetHooksCount() const;

        bool UseHook(int hook);

    private:
        InventoryComponent* inventory = nullptr;
        ItemEffectComponent* effects = nullptr;

        std::vector<std::string> hooks;
        float sinceUse = 0.f;

        bool IsValidHook(int hook) const;
        bool CanHang(const ItemDefinition& item) const;
        void HangOnFreeHook(const ItemDefinition& item);
        void ReadKeys();
    };
}
