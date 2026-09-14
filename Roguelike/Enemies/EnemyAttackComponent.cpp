#include "EnemyAttackComponent.h"
#include "AttackRules.h"
#include "ChaseComponent.h"
#include "GameSettings.h"
#include <DebugDraw.h>
#include <GameObject.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    EnemyAttackComponent::EnemyAttackComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
    }

    void EnemyAttackComponent::Start()
    {
        weapon = gameObject->GetComponent<WeaponComponent>();
        meleeWeapon = gameObject->GetComponent<MeleeWeaponComponent>();
        health = gameObject->GetComponent<HealthComponent>();
        chase = gameObject->GetComponent<ChaseComponent>();

        if (weapon == nullptr && meleeWeapon == nullptr)
        {
            LOG_ERROR("Enemy attack needs a weapon component on " + gameObject->GetName());
        }
    }

    bool EnemyAttackComponent::IsAttacking() const
    {
        return isAttacking;
    }

    void EnemyAttackComponent::Update(float deltaTime)
    {
        isAttacking = false;

        if (targetName.empty())
        {
            return;
        }

        XYZEngine::GameObject* target = XYZEngine::GameWorld::Instance()->FindGameObject(targetName);
        auto targetHealth = target != nullptr ? target->GetComponent<HealthComponent>() : nullptr;

        AttackSense sense;
        sense.isAlive = health == nullptr || health->IsAlive();
        sense.hasTarget = target != nullptr;
        sense.isTargetAlive = targetHealth == nullptr || targetHealth->IsAlive();
        sense.canSeeTarget = chase != nullptr && chase->CanSeeTarget();
        sense.attackRange = attackRange;

        XYZEngine::Vector2Df targetPosition = target != nullptr
            ? target->GetTransform()->GetWorldPosition()
            : XYZEngine::Vector2Df{0.f, 0.f};
        sense.distanceToTarget = (targetPosition - transform->GetWorldPosition()).GetLength();

        if (!MayAttack(sense))
        {
            return;
        }

        isAttacking = true;

        if (meleeWeapon != nullptr)
        {
            meleeWeapon->TryQuickAttack();
            return;
        }

        if (weapon != nullptr)
        {
            weapon->TryShootAt(targetPosition);
        }
    }

    void EnemyAttackComponent::Render()
    {
        if (XYZEngine::DebugDraw::Instance()->IsEnabled() && attackRange > 0.f)
        {
            XYZEngine::DebugDraw::Instance()->DrawCircle(transform->GetWorldPosition(), attackRange, DEBUG_ATTACK_RANGE_COLOR);
        }
    }

    void EnemyAttackComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
    }

    void EnemyAttackComponent::SetAttackRange(float newAttackRange)
    {
        attackRange = newAttackRange;
    }
}
