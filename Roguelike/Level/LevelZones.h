#pragma once

#include <string>
#include <vector>
#include "LevelData.h"

namespace RoguelikeGame
{
    struct LevelZone
    {
        std::string id;
        int minColumn = 0;
        int minRow = 0;
        int maxColumn = 0;
        int maxRow = 0;

        bool Contains(int column, int row) const
        {
            return column >= minColumn && column <= maxColumn && row >= minRow && row <= maxRow;
        }
    };

    std::vector<LevelZone> BuildZones(const LevelData& levelData);
    const LevelZone* FindZoneAt(const std::vector<LevelZone>& zones, int column, int row);
}
