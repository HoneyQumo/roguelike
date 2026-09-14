#include "MeleeWeaponComponent.h"
#include "AreaDamage.h"
#include <MathUtils.h>
#include <GameObject.h>
#include "HealthComponent.h"
#include <PhysicsSystem.h>
#include <LoggerRegistry.h>
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace XYZEngine;

namespace RoguelikeGame
{

    MeleeWeaponComponent::MeleeWeaponComponent(GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
    }

    void MeleeWeaponComponent::Start()
    {
        animation = gameObject->GetComponent<SpriteMovementAnimationComponent>();
        movement = gameObject->GetComponent<MovementComponent>();

        faction = gameObject->GetComponent<FactionComponent>();
    }

    void MeleeWeaponComponent::Update(float deltaTime)
    {
        cooldown.Tick(deltaTime);

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

    void MeleeWeaponComponent::SetLunge(const float* frameSpeeds, int framesCount, float peakSpeed)
    {
        lungeSpeeds = frameSpeeds;
        lungeFramesCount = framesCount;
        lungePeakSpeed = peakSpeed;
    }

    void MeleeWeaponComponent::SetDefinition(const MeleeDefinition* newDefinition)
    {
        definition = newDefinition;
    }
    const MeleeDefinition* MeleeWeaponComponent::GetDefinition() const
    {
        return definition;
    }

    XYZEngine::SubscriptionId MeleeWeaponComponent::SubscribeSwing(std::function<void(MeleeAttackKind)> onSwing)
    {
        return swingEvent.Subscribe(std::move(onSwing));
    }

    XYZEngine::SubscriptionId MeleeWeaponComponent::SubscribeStrike(std::function<void(MeleeAttackKind, int)> onStrike)
    {
        return strikeEvent.Subscribe(std::move(onStrike));
    }

    XYZEngine::SubscriptionId MeleeWeaponComponent::SubscribeHit(std::function<void(MeleeAttackKind, const Vector2Df&, const Vector2Df&)> onHit)
    {
        return hitEvent.Subscribe(std::move(onHit));
    }

    bool MeleeWeaponComponent::IsReady() const
    {
        return !isAttacking && cooldown.IsReady();
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

        BeginAttack(MeleeAttackKind::Quick);

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

        BeginAttack(MeleeAttackKind::Heavy);
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
        return transform->GetForward();
    }

    void MeleeWeaponComponent::BeginAttack(MeleeAttackKind kind)
    {
        currentKind = kind;
        isAttacking = true;
        isHolding = false;
        hasStruck = false;
        chargeTimer = 0.f;
        attackTimer = 0.f;

        swingEvent.Invoke(kind);
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
        float arcLimit = std::cos(ToRadians(0.5f * attack.arcDegrees));

        DamageSource source;
        source.kind = DamageKind::Melee;
        source.attackerId = gameObject->GetId();
        source.attackerName = gameObject->GetName();
        source.attackerFaction = faction == nullptr ? Faction::Neutral : faction->GetFaction();

        int hits = 0;
        for (const AreaTarget& target : QueryDamageArea(origin, attack.range, gameObject).targets)
        {
            if (!CanDamage(faction == nullptr ? Faction::Neutral : faction->GetFaction(), target.faction))
            {
                continue;
            }

            Vector2Df hitDirection = forward;
            if (target.distance > 0.f)
            {
                hitDirection = target.direction;
                if (forward.DotProduct(hitDirection) < arcLimit)
                {
                    continue;
                }
            }

            source.position = target.position;
            source.direction = hitDirection;

            target.health->TakeDamage(damage, source);
            hits++;

            hitEvent.Invoke(currentKind, target.position, hitDirection);
        }

        strikeEvent.Invoke(currentKind, hits);

        LOG_INFO(gameObject->GetName() + (currentKind == MeleeAttackKind::Quick ? " quick melee " : " heavy melee ")
                 + std::to_string(static_cast<int>(damage)) + " damage, targets hit: " + std::to_string(hits));
    }

    void MeleeWeaponComponent::Finish()
    {
        cooldown.Start(GetAttack().recovery);

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
