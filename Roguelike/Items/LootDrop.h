#pragma once

#include <string>
#include <Vector.h>

namespace XYZEngine
{
    class GameObject;
}

namespace RoguelikeGame
{
    class LootCatalog;
    class ItemCatalog;

    void DropLoot(const std::string& tableId, XYZEngine::GameObject* owner, const XYZEngine::Vector2Df& position);
    void DropLoot(const std::string& tableId, XYZEngine::GameObject* owner, const XYZEngine::Vector2Df& position,
        const LootCatalog& loot, const ItemCatalog& items, int (*roll)(int));
}
