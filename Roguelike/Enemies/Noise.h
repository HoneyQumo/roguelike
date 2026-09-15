#pragma once

#include <Vector.h>
#include "Faction.h"

namespace RoguelikeGame
{
    constexpr float WALL_MUFFLE = 0.45f;

    struct Noise
    {
        XYZEngine::Vector2Df position = {0.f, 0.f};
        float radius = 0.f;
        Faction from = Faction::Neutral;
    };

    inline float MuffledRadius(float radius, int wallsBetween)
    {
        float left = radius;
        for (int wall = 0; wall < wallsBetween; wall++)
        {
            left *= WALL_MUFFLE;
        }

        return left;
    }

    inline bool IsHeard(const Noise& noise, const XYZEngine::Vector2Df& listener, Faction listenerSide, int wallsBetween = 0)
    {
        if (noise.radius <= 0.f)
        {
            return false;
        }

        if (noise.from != Faction::Neutral && noise.from == listenerSide)
        {
            return false;
        }

        float reach = MuffledRadius(noise.radius, wallsBetween);

        return (listener - noise.position).GetLengthSquared() <= reach * reach;
    }

    void RaiseNoise(const Noise& noise);
}
