#include "CoverSpot.h"
#include "GameSettings.h"

using namespace XYZEngine;

namespace RoguelikeGame
{
    namespace
    {
        /**
        *	Укрытие с запасом, а не впритык.
        *
        *	Самая дешёвая клетка всегда лежит на краю тени: там врага снова видно от
        *	одного шага игрока, и он принимается перебегать. Проверяем вторую точку - на
        *	полтайла ближе к угрозе: если и она закрыта, клетка лежит вглубь тени.
        */
        bool IsHiddenWithMargin(const LevelGrid& grid, const Vector2Df& threat, const Vector2Df& place)
        {
            if (!grid.HasWallBetween(threat, place))
            {
                return false;
            }

            Vector2Df toThreat = threat - place;
            float distance = toThreat.GetLength();
            if (distance <= ENEMY_COVER_MARGIN)
            {
                return true;
            }

            return grid.HasWallBetween(threat, place + toThreat * (ENEMY_COVER_MARGIN / distance));
        }
    }
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
                if (!IsHiddenWithMargin(grid, threat, place))
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
