#include "HidingSpots.h"
#include <algorithm>
#include <cstdlib>

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
        const Vector2Df& from, int radius, std::size_t wanted, int minGap)
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

        if (found.empty())
        {
            return spots;
        }

        std::vector<Candidate> chosen;
        auto isFarEnough = [&chosen, minGap](const Candidate& candidate)
        {
            for (const Candidate& taken : chosen)
            {
                if (std::max(std::abs(taken.column - candidate.column), std::abs(taken.row - candidate.row)) < minGap)
                {
                    return false;
                }
            }

            return true;
        };

        std::size_t last = found.size() - 1u;
        for (std::size_t pick = 0u; pick < wanted && chosen.size() < wanted; pick++)
        {
            std::size_t target = last * (pick + 1u) / wanted;

            std::size_t taken = found.size();
            for (std::size_t step = 0u; step <= last; step++)
            {
                std::size_t behind = target >= step ? target - step : found.size();
                std::size_t ahead = target + step <= last ? target + step : found.size();

                if (behind < found.size() && isFarEnough(found[behind]))
                {
                    taken = behind;
                    break;
                }
                if (ahead < found.size() && isFarEnough(found[ahead]))
                {
                    taken = ahead;
                    break;
                }
            }

            if (taken >= found.size())
            {
                break;
            }

            chosen.push_back(found[taken]);
        }

        std::sort(chosen.begin(), chosen.end(), [](const Candidate& first, const Candidate& second)
        {
            return first.distance < second.distance;
        });

        for (const Candidate& candidate : chosen)
        {
            spots.push_back(grid.ToWorld(candidate.column, candidate.row));
        }

        return spots;
    }
}
