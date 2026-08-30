#include "Projectile.h"
#include "GameSettings.h"
#include "Fx.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>
#include <ProjectileComponent.h>
#include <LoggerRegistry.h>
#include <cmath>

namespace RoguelikeGame
{
    constexpr float DEGREES_IN_RADIAN = 57.29578f;

    void Projectile::Spawn(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction, float damage, float speed,
                           const std::string& shooterName, BulletKind bullet)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Projectile");
        gameObject->SetRenderLayer(EFFECT_RENDER_LAYER);

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetWorldPosition(position);

        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(BULLET_TEXTURE, static_cast<int>(bullet));
        if (texture == nullptr)
        {
            LOG_ERROR("Bullet texture is not loaded");
        }
        else
        {
            transform->SetWorldRotation(std::atan2(direction.y, direction.x) * DEGREES_IN_RADIAN);

            auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
            renderer->SetTexture(*texture);
            renderer->SetPixelSize(FX_BULLET.width, FX_BULLET.height);
            renderer->SetPivot(FX_BULLET.pivotX / FX_BULLET.width, FX_BULLET.pivotY / FX_BULLET.height);
        }

        auto body = gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
        body->SetKinematic(false);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(PROJECTILE_COLLIDER_SIZE, PROJECTILE_COLLIDER_SIZE);
        collider->SetTrigger(true);

        auto projectile = gameObject->AddComponent<XYZEngine::ProjectileComponent>();
        projectile->SetDirection(direction);
        projectile->SetSpeed(speed);
        projectile->SetDamage(damage);
        projectile->SetLifetime(PROJECTILE_LIFETIME);
        projectile->SetShooterName(shooterName);
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
    }
}
