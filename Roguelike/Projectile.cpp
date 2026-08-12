#include "Projectile.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <RectangleRendererComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>
#include <ProjectileComponent.h>

namespace RoguelikeGame
{
    void Projectile::Spawn(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction, float damage, float speed, const std::string& shooterName)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Projectile");

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::RectangleRendererComponent>();
        renderer->SetSize(PROJECTILE_SIZE, PROJECTILE_SIZE);
        renderer->SetColor(PROJECTILE_COLOR);

        auto body = gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
        body->SetKinematic(false);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(PROJECTILE_SIZE, PROJECTILE_SIZE);
        collider->SetTrigger(true);

        auto projectile = gameObject->AddComponent<XYZEngine::ProjectileComponent>();
        projectile->SetDirection(direction);
        projectile->SetSpeed(speed);
        projectile->SetDamage(damage);
        projectile->SetLifetime(PROJECTILE_LIFETIME);
        projectile->SetShooterName(shooterName);
    }
}
