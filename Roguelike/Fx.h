#pragma once

#include <string>
#include <Vector.h>
#include "SpriteAtlas.h"

namespace RoguelikeGame
{
    class Fx
    {
    public:
        static void SpawnBloodHit(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction);
        static void SpawnImpact(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction);

    private:
        static void Spawn(const std::string& textureMapName, const FxStrip& strip, const XYZEngine::Vector2Df& position, float angle);
        static float ToAngle(const XYZEngine::Vector2Df& direction);
    };
}
