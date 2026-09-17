#include "CoverSpot.h"

using namespace XYZEngine;

namespace RoguelikeGame
{
    bool FindCoverSpot(const LevelGrid& grid, const PathField& field, const Vector2Df& from,
        const Vector2Df& threat, int radius, Vector2Df& spot)
    {
        if (radius <= 0 || grid.IsEmpty() || field.IsEmpty())
        {
            return false;
        }

        int centreColumn = 0;
        int centreRow = 0;
        grid.ToCell(from, centreColumn, centreRow);

        bool hasBest = false;
        int bestCost = 0;
        Vector2Df best;

        for (int row = centreRow - radius; row <= centreRow + radius; row++)
        {
            for (int column = centreColumn - radius; column <= centreColumn + radius; column++)
            {
                if (!grid.IsPassable(column, row) || !field.IsReachable(column, row))
                {
                    continue;
                }

                Vector2Df place = grid.ToWorld(column, row);
                if (!grid.HasWallBetween(threat, place))
                {
                    continue;
                }

                int cost = field.GetDistance(column, row);
                if (hasBest && cost >= bestCost)
                {
                    continue;
                }

                hasBest = true;
                bestCost = cost;
                best = place;
            }
        }

        if (hasBest)
        {
            spot = best;
        }

        return hasBest;
    }
}
