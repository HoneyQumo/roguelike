#include "ItemCatalog.h"

namespace RoguelikeGame
{
    void ItemCatalog::Add(ItemDefinition definition)
    {
        items.push_back(std::move(definition));
    }

    const ItemDefinition* ItemCatalog::Find(const std::string& id) const
    {
        for (const ItemDefinition& item : items)
        {
            if (item.id == id)
            {
                return &item;
            }
        }

        return nullptr;
    }

    bool ItemCatalog::Contains(const std::string& id) const
    {
        return Find(id) != nullptr;
    }

    std::size_t ItemCatalog::Size() const
    {
        return items.size();
    }

    std::vector<ItemDefinition>::const_iterator ItemCatalog::begin() const
    {
        return items.begin();
    }

    std::vector<ItemDefinition>::const_iterator ItemCatalog::end() const
    {
        return items.end();
    }

    std::vector<ItemDefinition>::iterator ItemCatalog::begin()
    {
        return items.begin();
    }

    std::vector<ItemDefinition>::iterator ItemCatalog::end()
    {
        return items.end();
    }

    const ItemCatalog& ItemCatalog::Empty()
    {
        static const ItemCatalog empty;
        return empty;
    }
}
