#include "HidingSpots.h"
#include <algorithm>

using namespace XYZEngine;

namespace RoguelikeGame
{
    namespace
    {
        struct Candidate
        {
            int distance = 0;
            int column = 0;
            int row = 0;
        };
    }

    std::vector<Vector2Df> FindHidingSpots(const LevelGrid& grid, const PathField& field,
        const Vector2Df& from, int radius, std::size_t wanted)
    {
        std::vector<Vector2Df> spots;
        if (radius <= 0 || wanted == 0u || grid.IsEmpty() || field.IsEmpty())
        {
            return spots;
        }

        int centreColumn = 0;
        int centreRow = 0;
        grid.ToCell(from, centreColumn, centreRow);

        std::vector<Candidate> found;
        for (int row = centreRow - radius; row <= centreRow + radius; row++)
        {
            for (int column = centreColumn - radius; column <= centreColumn + radius; column++)
            {
                if (!grid.IsPassable(column, row) || !field.IsReachable(column, row))
                {
                    continue;
                }

                Vector2Df spot = grid.ToWorld(column, row);
                if (!grid.HasWallBetween(from, spot))
                {
                    continue;
                }

                found.push_back({field.GetDistance(column, row), column, row});
            }
        }

        std::sort(found.begin(), found.end(), [](const Candidate& first, const Candidate& second)
        {
            if (first.distance != second.distance)
            {
                return first.distance < second.distance;
            }
            if (first.row != second.row)
            {
                return first.row < second.row;
            }

            return first.column < second.column;
        });

        for (const Candidate& candidate : found)
        {
            if (spots.size() >= wanted)
            {
                break;
            }

            spots.push_back(grid.ToWorld(candidate.column, candidate.row));
        }

        return spots;
    }
}
