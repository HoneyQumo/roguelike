#include "EnemyAttackComponent.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    EnemyAttackComponent::EnemyAttackComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
    }

    void EnemyAttackComponent::Update(float deltaTime)
    {
        if (!areWeaponsSearched)
        {
            weapon = gameObject->GetComponent<XYZEngine::WeaponComponent>();
            meleeWeapon = gameObject->GetComponent<XYZEngine::MeleeWeaponComponent>();
            areWeaponsSearched = true;

            if (weapon == nullptr && meleeWeapon == nullptr)
            {
                LOG_ERROR("Enemy attack needs a weapon component on " + gameObject->GetName());
                gameObject->RemoveComponent(this);
                return;
            }
        }

        if (health == nullptr)
        {
            health = gameObject->GetComponent<XYZEngine::HealthComponent>();
        }

        if (health != nullptr && !health->IsAlive())
        {
            return;
        }

        if (targetName.empty() || attackRange <= 0.f)
        {
            return;
        }

        XYZEngine::GameObject* target = XYZEngine::GameWorld::Instance()->FindGameObject(targetName);
        if (target == nullptr)
        {
            return;
        }

        auto targetHealth = target->GetComponent<XYZEngine::HealthComponent>();
        if (targetHealth != nullptr && !targetHealth->IsAlive())
        {
            return;
        }

        XYZEngine::Vector2Df targetPosition = target->GetComponent<XYZEngine::TransformComponent>()->GetWorldPosition();
        if ((targetPosition - transform->GetWorldPosition()).GetLength() > attackRange)
        {
            return;
        }

        if (meleeWeapon != nullptr)
        {
            meleeWeapon->TryQuickAttack();
            return;
        }

        weapon->TryShootAt(targetPosition);
    }

    void EnemyAttackComponent::Render()
    {
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
