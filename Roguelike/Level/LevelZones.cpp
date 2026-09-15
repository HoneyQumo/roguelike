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

    bool AreNeighbours(const LevelZone& first, const LevelZone& second, int gap)
    {
        if (first.id == second.id)
        {
            return false;
        }

        bool isApartByColumn = first.minColumn - second.maxColumn > gap || second.minColumn - first.maxColumn > gap;
        bool isApartByRow = first.minRow - second.maxRow > gap || second.minRow - first.maxRow > gap;

        return !isApartByColumn && !isApartByRow;
    }

    std::vector<std::string> ZonesWithin(const std::vector<LevelZone>& zones, int column, int row, int ahead, int gap)
    {
        std::vector<std::string> found;

        const LevelZone* start = FindZoneAt(zones, column, row);
        if (start == nullptr)
        {
            return found;
        }

        std::vector<int> steps(zones.size(), -1);
        std::vector<std::size_t> wave;

        for (std::size_t index = 0u; index < zones.size(); index++)
        {
            if (zones[index].id == start->id)
            {
                steps[index] = 0;
                wave.push_back(index);
                found.push_back(zones[index].id);
            }
        }

        for (std::size_t at = 0u; at < wave.size(); at++)
        {
            std::size_t current = wave[at];
            if (steps[current] >= ahead)
            {
                continue;
            }

            for (std::size_t other = 0u; other < zones.size(); other++)
            {
                if (steps[other] >= 0 || !AreNeighbours(zones[current], zones[other], gap))
                {
                    continue;
                }

                steps[other] = steps[current] + 1;
                wave.push_back(other);
                found.push_back(zones[other].id);
            }
        }

        return found;
    }
}
