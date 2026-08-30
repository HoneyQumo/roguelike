#include "Fx.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <SpriteAnimationComponent.h>
#include <LoggerRegistry.h>
#include <randomizer.h>
#include <cmath>

namespace RoguelikeGame
{
    constexpr float DEGREES_IN_RADIAN = 57.29578f;

    constexpr int BLOOD_HIT_SPLASHES = 2;
    constexpr float BLOOD_HIT_SPREAD = 20.f;

    void Fx::SpawnBloodHit(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction)
    {
        float angle = ToAngle(direction);
        for (int i = 0; i < BLOOD_HIT_SPLASHES; i++)
        {
            Spawn(BLOOD_HIT_TEXTURE, FX_BLOOD_HIT, position, angle + random<float>(-BLOOD_HIT_SPREAD, BLOOD_HIT_SPREAD));
        }
    }

    // Искры летят назад по траектории
    void Fx::SpawnImpact(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction)
    {
        Spawn(IMPACT_TEXTURE, FX_IMPACT, position, ToAngle(direction) + 180.f);
    }

    void Fx::Spawn(const std::string& textureMapName, const FxStrip& strip, const XYZEngine::Vector2Df& position, float angle)
    {
        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(textureMapName, 0);
        if (texture == nullptr)
        {
            LOG_ERROR("Effect texture is not loaded: " + textureMapName);
            return;
        }

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Fx");

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetWorldPosition(position);
        transform->SetWorldRotation(angle);

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(strip.width, strip.height);
        renderer->SetPivot(strip.pivotX / strip.width, strip.pivotY / strip.height);

        auto animation = gameObject->AddComponent<XYZEngine::SpriteAnimationComponent>();
        animation->SetFrames(textureMapName, 0, strip.frames, FramesPerSecond(strip.millisecondsPerFrame));
        animation->SetEndBehaviour(XYZEngine::SpriteAnimationEnd::Destroy);
        animation->Play();
    }

    float Fx::ToAngle(const XYZEngine::Vector2Df& direction)
    {
        if (direction.GetLength() <= 0.f)
        {
            return 0.f;
        }

        return std::atan2(direction.y, direction.x) * DEGREES_IN_RADIAN;
    }
}
