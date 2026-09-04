#pragma once

#include <string>
#include <GameObject.h>
#include <SpriteRendererComponent.h>
#include <SpriteAnimationComponent.h>
#include <Vector.h>
#include "SpriteAtlas.h"

namespace RoguelikeGame
{
    class Fx
    {
    public:
        static void SpawnBloodHit(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction);
        static void SpawnImpact(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction);
        static void SpawnExplosion(const XYZEngine::Vector2Df& position, float radius);

        static XYZEngine::SpriteRendererComponent* AddSprite(XYZEngine::GameObject* gameObject, const std::string& textureMapName,
                                                             const FxStrip& strip, int frameIndex = 0, float scale = 1.f);
        static XYZEngine::SpriteAnimationComponent* AddAnimation(XYZEngine::GameObject* gameObject, const std::string& textureMapName,
                                                                 const FxStrip& strip);

    private:
        static void Spawn(const std::string& textureMapName, const FxStrip& strip, const XYZEngine::Vector2Df& position, float angle, float scale = 1.f);
        static float ToAngle(const XYZEngine::Vector2Df& direction);
    };
}
