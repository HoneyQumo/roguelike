#include "Fx.h"
#include "ParticleCatalog.h"
#include "Shake.h"
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
        ShakeEveryCamera(shake);
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

    // Клуб случайно повёрнут: одинаково лежащие клубы выдают один и тот же спрайт.
    void Fx::SpawnTireSmoke(const XYZEngine::Vector2Df& position, float scale)
    {
        Spawn(SMOKE_TEXTURE, FX_TIRE_SMOKE, position, random<float>(0.f, 360.f), scale);
    }

    void Fx::SpawnHit(const std::string& effect, const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction)
    {
        if (effect == "none")
        {
            return;
        }

        if (effect == "blood")
        {
            SpawnBloodHit(position, direction);
            return;
        }

        SpawnImpact(position, direction);
    }

    void Fx::SpawnPuppeteerRift(const XYZEngine::Vector2Df& position, float radius)
    {
        Spawn(PUPPETEER_RIFT_TEXTURE, FX_PUPPETEER_RIFT, position, 0.f,
              2.f * radius / static_cast<float>(FX_PUPPETEER_RIFT.width));
    }

    void Fx::SpawnPuppeteerCloud(const XYZEngine::Vector2Df& position, float radius)
    {
        Spawn(PUPPETEER_CLOUD_TEXTURE, FX_PUPPETEER_CLOUD, position, 0.f,
              2.f * radius / static_cast<float>(FX_PUPPETEER_CLOUD.width));
    }

    void Fx::SpawnStringSnap(const XYZEngine::Vector2Df& position)
    {
        Spawn(PUPPETEER_SNAP_TEXTURE, FX_PUPPETEER_SNAP, position, 0.f, 1.f);
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
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(FX_OBJECT_NAME);
        gameObject->SetTemporary(true);
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
