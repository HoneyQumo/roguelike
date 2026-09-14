#pragma once

#include <string>
#include <Component.h>
#include <TransformComponent.h>
#include "WeaponComponent.h"
#include "MeleeWeaponComponent.h"
#include "HealthComponent.h"
#include <Vector.h>

namespace RoguelikeGame
{
    class ChaseComponent;
}

namespace RoguelikeGame
{
    // Отвечает за аттаку. За направление отвечает AimRotationComponent.
    class EnemyAttackComponent : public XYZEngine::Component
    {
    public:
        EnemyAttackComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetTargetName(const std::string& newTargetName);
        void SetAttackRange(float newAttackRange);
        bool IsAttacking() const;

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        WeaponComponent* weapon = nullptr;
        MeleeWeaponComponent* meleeWeapon = nullptr;
        HealthComponent* health = nullptr;
        ChaseComponent* chase = nullptr;

        std::string targetName;
        float attackRange = 0.f;
        bool isAttacking = false;
    };
}
