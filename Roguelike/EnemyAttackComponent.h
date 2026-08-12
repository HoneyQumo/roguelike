#pragma once

#include <string>
#include <Component.h>
#include <TransformComponent.h>
#include <WeaponComponent.h>
#include <HealthComponent.h>
#include <SpriteRendererComponent.h>
#include <Vector.h>

namespace RoguelikeGame
{
    class EnemyAttackComponent : public XYZEngine::Component
    {
    public:
        EnemyAttackComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetTargetName(const std::string& newTargetName);
        void SetAttackRange(float newAttackRange);
        void SetWeapon(XYZEngine::TransformComponent* weaponTransform, XYZEngine::SpriteRendererComponent* weaponRenderer);

    private:
        XYZEngine::TransformComponent* transform;
        XYZEngine::WeaponComponent* weapon = nullptr;
        XYZEngine::HealthComponent* health = nullptr;

        XYZEngine::TransformComponent* weaponTransform = nullptr;
        XYZEngine::SpriteRendererComponent* weaponRenderer = nullptr;

        std::string targetName;
        float attackRange = 0.f;

        void AimWeapon(const XYZEngine::Vector2Df& direction);
    };
}
