#pragma once

#include <string>
#include "LevelCatalog.h"

namespace RoguelikeGame
{
    enum class LevelStepKind
    {
        Next,
        Finished,
        Unknown
    };

    struct LevelStep
    {
        LevelStepKind kind = LevelStepKind::Unknown;
        int index = -1;
    };

    LevelStep ResolveNextLevel(const LevelCatalog& levels, int currentIndex, const std::string& nextLevelId);
}
