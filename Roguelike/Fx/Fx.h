#pragma once

#include <string>
#include <CameraComponent.h>
#include <GameObject.h>
#include <SpriteRendererComponent.h>
#include <SpriteAnimationComponent.h>
#include <Vector.h>
#include "BossSpriteAtlas.h"
#include "SpriteAtlas.h"

namespace RoguelikeGame
{
    class Fx
    {
    public:
        static void ShakeCamera(const XYZEngine::CameraShake& shake);

        static void SpawnHealBurst(const XYZEngine::Vector2Df& position);
        static void SpawnHitBurst(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction);

        static void SpawnBloodHit(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction);
        static void SpawnImpact(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction);
        static void SpawnHit(const std::string& effect, const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction);
        static void SpawnExplosion(const XYZEngine::Vector2Df& position, float radius);
        static void SpawnTireSmoke(const XYZEngine::Vector2Df& position, float scale);

        static void SpawnPuppeteerRift(const XYZEngine::Vector2Df& position, float radius);
        static void SpawnPuppeteerCloud(const XYZEngine::Vector2Df& position, float radius);
        static void SpawnStringSnap(const XYZEngine::Vector2Df& position);

        static XYZEngine::SpriteRendererComponent* AddSprite(XYZEngine::GameObject* gameObject, const std::string& textureMapName,
                                                             const FxStrip& strip, int frameIndex = 0, float scale = 1.f);
        static XYZEngine::SpriteAnimationComponent* AddAnimation(XYZEngine::GameObject* gameObject, const std::string& textureMapName,
                                                                 const FxStrip& strip);

    private:
        static void Spawn(const std::string& textureMapName, const FxStrip& strip, const XYZEngine::Vector2Df& position, float angle, float scale = 1.f);
        static float ToAngle(const XYZEngine::Vector2Df& direction);
    };
}
