#pragma once

#include <Vector.h>

namespace RoguelikeGame
{
    // На обходе вокруг стены шаг и цель расходятся, а конус зрения едет за прицелом.
    inline XYZEngine::Vector2Df LookAheadPoint(const XYZEngine::Vector2Df& position,
        const XYZEngine::Vector2Df& step, const XYZEngine::Vector2Df& goal, float distance)
    {
        if (step.IsZero() || distance <= 0.f)
        {
            return goal;
        }

        return position + step.Normalized() * distance;
    }
}
