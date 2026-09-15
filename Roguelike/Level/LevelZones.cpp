#include "LevelZones.h"
#include <algorithm>

namespace RoguelikeGame
{
    std::vector<LevelZone> BuildZones(const LevelData& levelData)
    {
        std::vector<LevelZone> zones;

        for (const ZonePlacement& placement : levelData.zones)
        {
            if (placement.zoneId.empty())
            {
                continue;
            }

            auto found = std::find_if(zones.begin(), zones.end(),
                [&placement](const LevelZone& zone) { return zone.id == placement.zoneId; });

            if (found == zones.end())
            {
                zones.push_back({placement.zoneId, placement.column, placement.row, placement.column, placement.row});
                continue;
            }

            found->minColumn = std::min(found->minColumn, placement.column);
            found->minRow = std::min(found->minRow, placement.row);
            found->maxColumn = std::max(found->maxColumn, placement.column);
            found->maxRow = std::max(found->maxRow, placement.row);
        }

        return zones;
    }

    const LevelZone* FindZoneAt(const std::vector<LevelZone>& zones, int column, int row)
    {
        for (const LevelZone& zone : zones)
        {
            if (zone.Contains(column, row))
            {
                return &zone;
            }
        }

        return nullptr;
    }
}
