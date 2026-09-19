#pragma once

#include <GameObject.h>
#include "HudScreen.h"
#include "InventoryScreen.h"
#include "FadeScreen.h"
#include "MessageScreen.h"
#include "SubtitleScreen.h"

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateUiRoot(HudScreen& hud, SubtitleScreen& subtitles, InventoryScreen& inventory,
                                        MessageScreen& message, FadeScreen& fade);
}
