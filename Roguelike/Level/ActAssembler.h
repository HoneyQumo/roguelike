#pragma once

#include <functional>
#include <string>
#include "ActPlan.h"
#include "LevelData.h"

namespace RoguelikeGame
{
    using RoomSource = std::function<const LevelData*(const std::string&)>;

    LevelData AssembleAct(const ActPlan& plan, const RoomSource& rooms);

    LevelData LoadAct(const std::string& filePath);
}
