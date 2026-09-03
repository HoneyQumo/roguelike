#pragma once

#include <Vector.h>

namespace RoguelikeGame
{
    class BloodPool
    {
    public:
        static void Spawn(const XYZEngine::Vector2Df& position, float angle);
    };
}
