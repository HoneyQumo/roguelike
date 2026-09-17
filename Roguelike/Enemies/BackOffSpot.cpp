#include "BackOffSpot.h"
#include "GameSettings.h"
#include <algorithm>

using namespace XYZEngine;

namespace RoguelikeGame
{
    namespace
    {
        // Выигрыш меряю ступенями по полтайла: без округления равных клеток не
        // бывает, и цена хода до второго места в сравнении просто не доходит.
        int GainOf(const Vector2Df& place, const Vector2Df& threat, float wanted)
        {
            float away = std::min((place - threat).GetLength(), wanted);

            return static_cast<int>(away / BACK_OFF_GAIN_STEP);
        }

        /**
        *	Клетка за спиной у угрозы формально «дальше», но идти туда - значит
        *	пройти сквозь неё. Отход должен уводить в сторону, а не в обход.
        */
        bool LeadsAway(const Vector2Df& from, const Vector2Df& threat, const Vector2Df& place)
        {
            Vector2Df back = from - threat;
            Vector2Df step = place - from;

            float backLength = back.GetLength();
            float stepLength = step.GetLength();
            if (backLength <= 0.f || stepLength <= 0.f)
            {
                return true;
            }

            return (back.x * step.x + back.y * step.y) / (backLength * stepLength) >= BACK_OFF_COSINE;
        }
    }

    bool FindBackOffSpot(const LevelGrid& grid, const PathField& field, const Vector2Df& from,
        const Vector2Df& threat, float wanted, int radius, Vector2Df& spot)
    {
        if (radius <= 0 || wanted <= 0.f || grid.IsEmpty() || field.IsEmpty())
        {
            return false;
        }

        int centreColumn = 0;
        int centreRow = 0;
        grid.ToCell(from, centreColumn, centreRow);

        float here = (from - threat).GetLength();

        bool hasBest = false;
        int bestGain = 0;
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
                if ((place - threat).GetLength() <= here || !LeadsAway(from, threat, place))
                {
                    continue;
                }

                int gain = GainOf(place, threat, wanted);
                int cost = field.GetDistance(column, row);

                // Равные по выигрышу решает цена хода, равные по обоим - порядок обхода.
                if (hasBest && (gain < bestGain || (gain == bestGain && cost >= bestCost)))
                {
                    continue;
                }

                hasBest = true;
                bestGain = gain;
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
