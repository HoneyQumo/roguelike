#pragma once

#include <string>
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

        static const ItemCatalog& Empty();

    private:
        std::vector<ItemDefinition> items;
    };
}
