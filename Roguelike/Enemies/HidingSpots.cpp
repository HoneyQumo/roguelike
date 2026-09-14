#include "HidingSpots.h"
#include "GameSettings.h"
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
            float offAngle = 0.f;
        };
    }

    bool IsJustBehindACorner(const LevelGrid& grid, const Vector2Df& from, int column, int row)
    {
        if (!grid.IsPassable(column, row) || !grid.HasWallBetween(from, grid.ToWorld(column, row)))
        {
            return false;
        }

        constexpr int STEP_COLUMNS[4] = {1, -1, 0, 0};
        constexpr int STEP_ROWS[4] = {0, 0, 1, -1};

        for (int side = 0; side < 4; side++)
        {
            int neighbourColumn = column + STEP_COLUMNS[side];
            int neighbourRow = row + STEP_ROWS[side];

            if (grid.IsPassable(neighbourColumn, neighbourRow)
                && !grid.HasWallBetween(from, grid.ToWorld(neighbourColumn, neighbourRow)))
            {
                return true;
            }
        }

        return false;
    }

    std::vector<Vector2Df> FindHidingSpots(const LevelGrid& grid, const PathField& field,
        const Vector2Df& from, const Vector2Df& escape, int radius, std::size_t wanted, int minGap)
    {
        std::vector<Vector2Df> spots;
        if (radius <= 0 || wanted == 0u || grid.IsEmpty() || field.IsEmpty())
        {
            return spots;
        }

        int centreColumn = 0;
        int centreRow = 0;
        grid.ToCell(from, centreColumn, centreRow);

        bool hasEscape = escape.GetLengthSquared() > 0.f;
        Vector2Df heading = hasEscape ? escape.Normalized() : Vector2Df{0.f, 0.f};

        std::vector<Candidate> found;
        for (int row = centreRow - radius; row <= centreRow + radius; row++)
        {
            for (int column = centreColumn - radius; column <= centreColumn + radius; column++)
            {
                if (!field.IsReachable(column, row) || !IsJustBehindACorner(grid, from, column, row))
                {
                    continue;
                }

                Candidate candidate;
                candidate.distance = field.GetDistance(column, row);
                candidate.column = column;
                candidate.row = row;
                candidate.offAngle = 0.f;

                if (hasEscape)
                {
                    Vector2Df toSpot = grid.ToWorld(column, row) - from;
                    float length = toSpot.GetLength();
                    if (length <= 0.f)
                    {
                        continue;
                    }

                    float cosine = (heading.x * toSpot.x + heading.y * toSpot.y) / length;
                    if (cosine < SEARCH_ESCAPE_COSINE)
                    {
                        continue;
                    }

                    candidate.offAngle = 1.f - cosine;
                }

                found.push_back(candidate);
            }
        }

        std::sort(found.begin(), found.end(), [](const Candidate& first, const Candidate& second)
        {
            if (first.distance != second.distance)
            {
                return first.distance < second.distance;
            }
            if (first.offAngle != second.offAngle)
            {
                return first.offAngle < second.offAngle;
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
