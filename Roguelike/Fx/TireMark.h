#pragma once

#include <Vector.h>

namespace RoguelikeGame
{
    class TireMark
    {
    public:
        /**
        *	Чёрный след на асфальте. Угол - направление, в котором машину тащит,
        *	а не то, куда смотрит её нос: в заносе это разные вещи, и след должен
        *	лечь вдоль движения.
        */
        static void Spawn(const XYZEngine::Vector2Df& position, float angle);
    };
}
