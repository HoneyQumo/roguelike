#include "pch.h"
#include "MeleeWeaponComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "PhysicsSystem.h"
#include "LoggerRegistry.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace XYZEngine
{
    constexpr float RADIANS_IN_DEGREE = 0.01745329f;

    MeleeWeaponComponent::MeleeWeaponComponent(GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetComponent<TransformComponent>();
    }

    void MeleeWeaponComponent::Update(float deltaTime)
    {
        if (!areComponentsSearched)
        {
            animation = gameObject->GetComponent<SpriteMovementAnimationComponent>();
            movement = gameObject->GetComponent<MovementComponent>();
            areComponentsSearched = true;
        }

        if (cooldownTimer > 0.f)
        {
            cooldownTimer = std::max(0.f, cooldownTimer - deltaTime);
        }

        if (!isAttacking)
        {
            return;
        }

        attackTimer += deltaTime;

        if (currentKind == MeleeAttackKind::Heavy && isHolding)
        {
            chargeTimer += deltaTime;
            if (animation != nullptr)
            {
                animation->SetHeavyCharged(IsCharged());
            }
        }

        if (animation == nullptr)
        {
            if (currentKind == MeleeAttackKind::Heavy && isHolding)
            {
                return;
            }

            if (!hasStruck)
            {
                if (attackTimer >= GetAttack().windup)
                {
                    Strike();
                }
                return;
            }

            Finish();
            return;
        }

        MovementAnimation expected = currentKind == MeleeAttackKind::Quick ? MovementAnimation::Melee : MovementAnimation::Heavy;
        if (animation->GetCurrentAnimation() != expected)
        {
            Finish();
            return;
        }

        if (!hasStruck && animation->GetCurrentFrame() >= GetAttack().hitFrame)
        {
            Strike();
        }

        UpdateLunge(deltaTime);

        if (animation->IsFinished())
        {
            Finish();
        }
    }

    void MeleeWeaponComponent::Render()
    {
    }

    void MeleeWeaponComponent::SetQuickAttack(const MeleeAttack& newQuickAttack)
    {
        assert(newQuickAttack.range > 0.f);
        quickAttack = newQuickAttack;
    }

    void MeleeWeaponComponent::SetHeavyAttack(const MeleeAttack& newHeavyAttack)
    {
        assert(newHeavyAttack.range > 0.f);
        heavyAttack = newHeavyAttack;
    }

    void MeleeWeaponComponent::SetChargeTime(float newChargeTime)
    {
        assert(newChargeTime > 0.f);
        chargeTime = newChargeTime;
    }

    void MeleeWeaponComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
    }

    void MeleeWeaponComponent::SetLunge(const float* frameSpeeds, int framesCount, float peakSpeed)
    {
        lungeSpeeds = frameSpeeds;
        lungeFramesCount = framesCount;
        lungePeakSpeed = peakSpeed;
    }

    void MeleeWeaponComponent::SetSwingAction(std::function<void(MeleeAttackKind)> newSwingAction)
    {
        swingAction = newSwingAction;
    }

    void MeleeWeaponComponent::SetStrikeAction(std::function<void(MeleeAttackKind, int)> newStrikeAction)
    {
        strikeAction = newStrikeAction;
    }

    void MeleeWeaponComponent::SetHitAction(std::function<void(MeleeAttackKind, const Vector2Df&, const Vector2Df&)> newHitAction)
    {
        hitAction = newHitAction;
    }

    bool MeleeWeaponComponent::IsReady() const
    {
        return !isAttacking && cooldownTimer <= 0.f;
    }

    bool MeleeWeaponComponent::IsAttacking() const
    {
        return isAttacking;
    }

    bool MeleeWeaponComponent::IsCharging() const
    {
        return isAttacking && currentKind == MeleeAttackKind::Heavy && isHolding;
    }

    bool MeleeWeaponComponent::IsCharged() const
    {
        return chargeTimer >= chargeTime;
    }

    float MeleeWeaponComponent::GetChargeProgress() const
    {
        if (chargeTime <= 0.f)
        {
            return 1.f;
        }

        return std::min(chargeTimer / chargeTime, 1.f);
    }

    bool MeleeWeaponComponent::TryQuickAttack()
    {
        if (!IsReady())
        {
            return false;
        }

        Start(MeleeAttackKind::Quick);

        if (animation != nullptr)
        {
            animation->PlayMelee();
        }

        return true;
    }

    bool MeleeWeaponComponent::TryStartHeavyAttack()
    {
        if (!IsReady())
        {
            return false;
        }

        Start(MeleeAttackKind::Heavy);
        isHolding = true;

        if (animation != nullptr)
        {
            animation->PlayHeavy();
        }

        SetMovementEnabled(false);
        return true;
    }

    void MeleeWeaponComponent::ReleaseHeavyAttack()
    {
        if (!isAttacking || currentKind != MeleeAttackKind::Heavy || !isHolding)
        {
            return;
        }

        isHolding = false;
        attackTimer = 0.f;

        if (animation != nullptr)
        {
            animation->ReleaseHeavy();
        }
    }

    void MeleeWeaponComponent::CancelAttack()
    {
        if (!isAttacking)
        {
            return;
        }

        if (animation != nullptr)
        {
            animation->ReleaseHeavy();
        }

        Finish();
    }

    const MeleeAttack& MeleeWeaponComponent::GetAttack() const
    {
        return currentKind == MeleeAttackKind::Quick ? quickAttack : heavyAttack;
    }

    Vector2Df MeleeWeaponComponent::GetForward() const
    {
        float rotation = transform->GetWorldRotation() * RADIANS_IN_DEGREE;
        return {std::cos(rotation), std::sin(rotation)};
    }

    void MeleeWeaponComponent::Start(MeleeAttackKind kind)
    {
        currentKind = kind;
        isAttacking = true;
        isHolding = false;
        hasStruck = false;
        chargeTimer = 0.f;
        attackTimer = 0.f;

        if (swingAction != nullptr)
        {
            swingAction(kind);
        }
    }

    void MeleeWeaponComponent::Strike()
    {
        hasStruck = true;

        const MeleeAttack& attack = GetAttack();
        float damage = attack.damage;
        if (currentKind == MeleeAttackKind::Heavy)
        {
            damage += (attack.chargedDamage - attack.damage) * GetChargeProgress();
        }

        Vector2Df origin = transform->GetWorldPosition();
        Vector2Df forward = GetForward();
        float arcLimit = std::cos(0.5f * attack.arcDegrees * RADIANS_IN_DEGREE);

        sf::FloatRect area(origin.x - attack.range, origin.y - attack.range, 2.f * attack.range, 2.f * attack.range);

        int hits = 0;
        for (auto collider : PhysicsSystem::Instance()->Overlap(area))
        {
            GameObject* target = collider->GetGameObject();
            if (target == nullptr || target == gameObject)
            {
                continue;
            }

            if (!targetName.empty() && target->GetName() != targetName)
            {
                continue;
            }

            auto health = target->GetComponent<HealthComponent>();
            if (health == nullptr || !health->IsAlive())
            {
                continue;
            }

            auto targetTransform = target->GetComponent<TransformComponent>();
            if (targetTransform == nullptr)
            {
                continue;
            }

            Vector2Df targetPosition = targetTransform->GetWorldPosition();
            Vector2Df toTarget = targetPosition - origin;
            float distance = toTarget.GetLength();
            if (distance > attack.range)
            {
                continue;
            }

            Vector2Df hitDirection = forward;
            if (distance > 0.f)
            {
                hitDirection = (1.f / distance) * toTarget;
                if (forward.DotProduct(hitDirection) < arcLimit)
                {
                    continue;
                }
            }

            health->TakeDamage(damage);
            hits++;

            if (hitAction != nullptr)
            {
                hitAction(currentKind, targetPosition, hitDirection);
            }
        }

        if (strikeAction != nullptr)
        {
            strikeAction(currentKind, hits);
        }

        LOG_INFO(gameObject->GetName() + (currentKind == MeleeAttackKind::Quick ? " quick melee " : " heavy melee ")
                 + std::to_string(static_cast<int>(damage)) + " damage, targets hit: " + std::to_string(hits));
    }

    void MeleeWeaponComponent::Finish()
    {
        cooldownTimer = GetAttack().recovery;

        isAttacking = false;
        isHolding = false;
        hasStruck = false;
        chargeTimer = 0.f;
        attackTimer = 0.f;

        SetMovementEnabled(true);
    }

    void MeleeWeaponComponent::UpdateLunge(float deltaTime)
    {
        if (lungeSpeeds == nullptr || currentKind != MeleeAttackKind::Heavy || animation == nullptr)
        {
            return;
        }

        int frame = animation->GetCurrentFrame();
        if (frame < 0 || frame >= lungeFramesCount || lungeSpeeds[frame] <= 0.f)
        {
            return;
        }

        transform->MoveBy(lungePeakSpeed * lungeSpeeds[frame] * deltaTime * GetForward());
    }

    void MeleeWeaponComponent::SetMovementEnabled(bool isEnabled)
    {
        if (movement != nullptr)
        {
            movement->SetEnabled(isEnabled);
        }
    }
}
