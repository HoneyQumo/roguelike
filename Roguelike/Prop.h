#pragma once

#include <GameObject.h>
#include <Vector.h>
#include "PropCatalog.h"

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateProp(const PropDefinition& definition, const XYZEngine::Vector2Df& position);
}
