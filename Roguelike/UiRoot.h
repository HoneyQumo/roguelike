#pragma once

#include <GameObject.h>
#include "HudScreen.h"
#include "InventoryScreen.h"
#include "MessageScreen.h"

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateUiRoot(HudScreen& hud, InventoryScreen& inventory, MessageScreen& message);
}
