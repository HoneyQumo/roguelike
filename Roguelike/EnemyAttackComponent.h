#pragma once

#include <string>
#include <Component.h>
#include <TransformComponent.h>
#include <WeaponComponent.h>
#include <HealthComponent.h>
#include <Vector.h>

namespace RoguelikeGame
{
    // Отвечает за аттаку. За направление отвечает AimRotationComponent.
    class EnemyAttackComponent : public XYZEngine::Component
    {
    public:
        EnemyAttackComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetTargetName(const std::string& newTargetName);
        void SetAttackRange(float newAttackRange);

    private:
        XYZEngine::TransformComponent* transform;
        XYZEngine::WeaponComponent* weapon = nullptr;
        XYZEngine::HealthComponent* health = nullptr;

        std::string targetName;
        float attackRange = 0.f;
    };
}
