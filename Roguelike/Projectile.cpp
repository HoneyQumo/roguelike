#include "Projectile.h"
#include "GameSettings.h"
#include "Fx.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <SpriteAnimationComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>
#include <ProjectileComponent.h>
#include <ExplosiveComponent.h>
#include <LoggerRegistry.h>
#include <cmath>

namespace RoguelikeGame
{
    constexpr float DEGREES_IN_RADIAN = 57.29578f;

    void Projectile::Spawn(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction, float damage, float speed,
                           const std::string& shooterName, WeaponId weapon)
    {
        const WeaponDefinition& definition = GetWeapon(weapon);
        const ExplosiveDefinition* explosive = FindExplosive(weapon);

        bool isRocket = definition.bullet == BulletKind::Rocket;
        const FxStrip& strip = isRocket ? FX_ROCKET : FX_BULLET;
        const char* textureName = isRocket ? ROCKET_TEXTURE : BULLET_TEXTURE;
        int frameIndex = isRocket ? 0 : static_cast<int>(definition.bullet);

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(isRocket ? "Rocket" : "Projectile");
        gameObject->SetRenderLayer(EFFECT_RENDER_LAYER);

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetWorldPosition(position);

        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(textureName, frameIndex);
        if (texture == nullptr)
        {
            LOG_ERROR(std::string("Projectile texture is not loaded: ") + textureName);
        }
        else
        {
            transform->SetWorldRotation(std::atan2(direction.y, direction.x) * DEGREES_IN_RADIAN);

            auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
            renderer->SetTexture(*texture);
            float spriteScale = isRocket ? ROCKET_SPRITE_SCALE : 1.f;
            renderer->SetPixelSize(static_cast<int>(strip.width * spriteScale), static_cast<int>(strip.height * spriteScale));
            renderer->SetPivot(strip.pivotX / strip.width, strip.pivotY / strip.height);

            if (isRocket)
            {
                auto flight = gameObject->AddComponent<XYZEngine::SpriteAnimationComponent>();
                flight->SetFrames(textureName, 0, strip.frames, FramesPerSecond(strip.millisecondsPerFrame));
                flight->SetLooped(true);
                flight->Play();
            }
        }

        auto body = gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
        body->SetKinematic(false);

        float colliderSize = explosive == nullptr ? PROJECTILE_COLLIDER_SIZE : explosive->colliderSize;

        float directionLength = direction.GetLength();
        XYZEngine::Vector2Df bodyOffset = {0.f, 0.f};
        if (isRocket && directionLength > 0.f)
        {
            bodyOffset = (ROCKET_BODY_OFFSET / directionLength) * direction;
        }

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(colliderSize, colliderSize);
        collider->SetOffset(bodyOffset.x, bodyOffset.y);
        collider->SetTrigger(true);

        auto projectile = gameObject->AddComponent<XYZEngine::ProjectileComponent>();
        projectile->SetDirection(direction);
        projectile->SetSpeed(speed);
        projectile->SetShooterName(shooterName);

        if (explosive == nullptr)
        {
            projectile->SetDamage(damage);
            projectile->SetLifetime(PROJECTILE_LIFETIME);
            projectile->SetHitAction([](const XYZEngine::Vector2Df& hitPosition, const XYZEngine::Vector2Df& hitDirection, bool isCharacterHit)
            {
                if (isCharacterHit)
                {
                    Fx::SpawnBloodHit(hitPosition, hitDirection);
                }
                else
                {
                    Fx::SpawnImpact(hitPosition, hitDirection);
                }
            });

            return;
        }

        projectile->SetDamage(0.f);
        projectile->SetLifetime(explosive->lifetime);

        float blastRadius = explosive->radius;

        auto blast = gameObject->AddComponent<XYZEngine::ExplosiveComponent>();
        blast->SetRadius(blastRadius);
        blast->SetCoreRadius(EXPLOSION_CORE_RADIUS);
        blast->SetCenterDamage(damage);
        blast->SetEdgeDamagePart(explosive->edgeDamagePart);
        blast->SetSelfDamagePart(explosive->selfDamagePart);
        blast->SetOwnerName(shooterName);
        blast->SetExplodeAction([blastRadius](const XYZEngine::Vector2Df& blastPosition)
        {
            Fx::SpawnExplosion(blastPosition, blastRadius);
        });
        blast->SetHitAction([](const XYZEngine::Vector2Df& targetPosition, const XYZEngine::Vector2Df& hitDirection)
        {
            Fx::SpawnBloodHit(targetPosition, hitDirection);
        });

        projectile->SetHitAction([blast, bodyOffset](const XYZEngine::Vector2Df& hitPosition, const XYZEngine::Vector2Df& hitDirection, bool isCharacterHit)
        {
            blast->Explode(hitPosition + bodyOffset);
        });

        projectile->SetExpireAction([blast, bodyOffset](const XYZEngine::Vector2Df& expirePosition)
        {
            blast->Explode(expirePosition + bodyOffset);
        });
    }
}
