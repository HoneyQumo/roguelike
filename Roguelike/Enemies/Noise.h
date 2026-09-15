#pragma once

#include <Vector.h>
#include "Faction.h"

namespace RoguelikeGame
{
    struct Noise
    {
        XYZEngine::Vector2Df position = {0.f, 0.f};
        float radius = 0.f;
        Faction from = Faction::Neutral;
    };

    inline bool IsHeard(const Noise& noise, const XYZEngine::Vector2Df& listener, Faction listenerSide)
    {
        if (noise.radius <= 0.f)
        {
            return false;
        }

        if (noise.from != Faction::Neutral && noise.from == listenerSide)
        {
            return false;
        }

        return (listener - noise.position).GetLengthSquared() <= noise.radius * noise.radius;
    }

    void RaiseNoise(const Noise& noise);
}
