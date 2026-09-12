#include "Fx.h"
#include "ParticleCatalog.h"
#include <MathUtils.h>
#include <ParticleSystem.h>
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

    void Fx::SpawnHealBurst(const XYZEngine::Vector2Df& position)
    {
        const XYZEngine::ParticleSpec* spec = FindParticleSpec(ParticleEffect::HealBurst);
        if (spec == nullptr)
        {
            return;
        }

        XYZEngine::ParticleSystem::Instance()->Emit(*spec, position, {0.f, 1.f});
    }

    void Fx::SpawnHitBurst(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction)
    {
        const XYZEngine::ParticleSpec* spec = FindParticleSpec(ParticleEffect::HitBurst);
        if (spec == nullptr)
        {
            return;
        }

        XYZEngine::ParticleSystem::Instance()->Emit(*spec, position, direction.IsZero() ? XYZEngine::Vector2Df{0.f, 1.f} : direction);
    }

    void Fx::ShakeCamera(const XYZEngine::CameraShake& shake)
    {
        auto camera = XYZEngine::GameWorld::Instance()->FindComponent<XYZEngine::CameraComponent>(PLAYER_OBJECT_NAME);
        if (camera == nullptr)
        {
            return;
        }

        camera->Shake(shake);
    }

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

    void Fx::SpawnExplosion(const XYZEngine::Vector2Df& position, float radius)
    {
        Spawn(EXPLOSION_TEXTURE, FX_EXPLOSION, position, 0.f, 2.f * radius / static_cast<float>(FX_EXPLOSION.width));
    }

    XYZEngine::SpriteRendererComponent* Fx::AddSprite(XYZEngine::GameObject* gameObject, const std::string& textureMapName,
                                                      const FxStrip& strip, int frameIndex, float scale)
    {
        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(textureMapName, frameIndex);
        if (texture == nullptr)
        {
            LOG_ERROR("Effect texture is not loaded: " + textureMapName);
            return nullptr;
        }

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(static_cast<int>(strip.width * scale), static_cast<int>(strip.height * scale));
        renderer->SetPivot(strip.pivotX / strip.width, strip.pivotY / strip.height);

        return renderer;
    }

    XYZEngine::SpriteAnimationComponent* Fx::AddAnimation(XYZEngine::GameObject* gameObject, const std::string& textureMapName,
                                                          const FxStrip& strip)
    {
        auto animation = gameObject->AddComponent<XYZEngine::SpriteAnimationComponent>();
        animation->SetFrames(textureMapName, 0, strip.frames, strip.secondsPerFrame);

        return animation;
    }

    void Fx::Spawn(const std::string& textureMapName, const FxStrip& strip, const XYZEngine::Vector2Df& position, float angle, float scale)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Fx");
        gameObject->SetRenderLayer(EFFECT_RENDER_LAYER);

        if (AddSprite(gameObject, textureMapName, strip, 0, scale) == nullptr)
        {
            XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
            return;
        }

        auto transform = gameObject->GetTransform();
        transform->SetWorldPosition(position);
        transform->SetWorldRotation(angle);

        auto animation = AddAnimation(gameObject, textureMapName, strip);
        animation->SetEndBehaviour(XYZEngine::SpriteAnimationEnd::Destroy);
        animation->Play();
    }

    float Fx::ToAngle(const XYZEngine::Vector2Df& direction)
    {
        if (direction.IsZero())
        {
            return 0.f;
        }

        return XYZEngine::DegreesFromDirection(direction);
    }
}
