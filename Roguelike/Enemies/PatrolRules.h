#pragma once

#include <cstddef>
#include <vector>
#include <Vector.h>

namespace RoguelikeGame
{
    constexpr std::size_t NextPatrolIndex(std::size_t index, std::size_t count)
    {
        if (count == 0u)
        {
            return 0u;
        }

        return (index + 1u) % count;
    }

    inline std::size_t NearestPatrolIndex(const std::vector<XYZEngine::Vector2Df>& points, const XYZEngine::Vector2Df& position)
    {
        std::size_t best = 0u;
        float bestDistance = -1.f;

        for (std::size_t index = 0u; index < points.size(); index++)
        {
            float distance = (points[index] - position).GetLengthSquared();
            if (bestDistance < 0.f || distance < bestDistance)
            {
                bestDistance = distance;
                best = index;
            }
        }

        return best;
    }

    inline bool HasReachedPatrolPoint(const XYZEngine::Vector2Df& point, const XYZEngine::Vector2Df& position, float arriveDistance)
    {
        return (point - position).GetLength() <= arriveDistance;
    }
}
