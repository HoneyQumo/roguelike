#pragma once

#include <string>
#include <GameObject.h>
#include <Vector.h>
#include "ItemCatalog.h"

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateDoor(const std::string& doorId, const XYZEngine::Vector2Df& position,
        const ItemCatalog& items = ItemCatalog::Empty());
}
