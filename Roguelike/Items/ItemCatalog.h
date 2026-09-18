#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "ItemDefinition.h"

namespace RoguelikeGame
{
    class ItemCatalog
    {
    public:
        void Add(ItemDefinition definition);

        const ItemDefinition* Find(const std::string& id) const;
        bool Contains(const std::string& id) const;
        std::size_t Size() const;

        std::vector<ItemDefinition>::const_iterator begin() const;
        std::vector<ItemDefinition>::const_iterator end() const;

        std::vector<ItemDefinition>::iterator begin();
        std::vector<ItemDefinition>::iterator end();

        static const ItemCatalog& Empty();

    private:
        std::vector<ItemDefinition> items;
    };

    /**
    *	Обратный путь: по стволу найти предмет, которым его кладут в сумку.
    *	Ствол задан строкой, чтобы слой предметов не знал про каталог оружия.
    */
    inline const ItemDefinition* FindWeaponItem(const ItemCatalog& catalog, std::string_view weaponId)
    {
        for (const ItemDefinition& item : catalog)
        {
            if (item.effect.kind == ItemEffectKind::EquipWeapon && item.effect.target == weaponId)
            {
                return &item;
            }
        }

        return nullptr;
    }
}
