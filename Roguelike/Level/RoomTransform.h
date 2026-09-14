#pragma once

#include "LevelData.h"

namespace RoguelikeGame
{
    constexpr int ROOM_QUARTERS = 4;

    LevelData TransformRoom(const LevelData& room, int quarters, bool isMirrored);
}
