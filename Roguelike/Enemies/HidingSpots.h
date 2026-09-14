#pragma once

#include <vector>
#include <Vector.h>
#include "LevelGrid.h"
#include "PathField.h"

namespace RoguelikeGame
{
    std::vector<XYZEngine::Vector2Df> FindHidingSpots(const LevelGrid& grid, const PathField& field,
        const XYZEngine::Vector2Df& from, int radius, std::size_t wanted, int minGap);
}
