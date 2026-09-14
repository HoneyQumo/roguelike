#pragma once

#include <GameObject.h>
#include <Vector.h>
#include "ItemDefinition.h"

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateItem(const ItemDefinition& definition, const XYZEngine::Vector2Df& position,
        XYZEngine::GameObject* parent = nullptr);
    std::string ItemTextureName(const std::string& itemId);
}
