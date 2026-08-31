#pragma once

#include <functional>
#include <string>
#include "Component.h"
#include "TransformComponent.h"
#include "MovementComponent.h"
#include "SpriteMovementAnimationComponent.h"
#include "Vector.h"

namespace XYZEngine
{
    enum class MeleeAttackKind
    {
        Quick,
        Heavy
    };

    struct MeleeAttack
    {
        float damage = 10.f;
        float chargedDamage = 10.f;
        float range = 50.f;
        float arcDegrees = 90.f;
        float recovery = 0.2f;
        float windup = 0.15f;
        int hitFrame = 0;
    };

    class MeleeWeaponComponent : public Component
    {
    public:
        MeleeWeaponComponent(GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetQuickAttack(const MeleeAttack& newQuickAttack);
        void SetHeavyAttack(const MeleeAttack& newHeavyAttack);
        void SetChargeTime(float newChargeTime);
        void SetTargetName(const std::string& newTargetName);
        void SetLunge(const float* frameSpeeds, int framesCount, float peakSpeed);

        void SetSwingAction(std::function<void(MeleeAttackKind)> newSwingAction);
        void SetStrikeAction(std::function<void(MeleeAttackKind, int)> newStrikeAction);
        void SetHitAction(std::function<void(MeleeAttackKind, const Vector2Df&, const Vector2Df&)> newHitAction);

        bool IsReady() const;
        bool IsAttacking() const;
        bool IsCharging() const;
        bool IsCharged() const;
        float GetChargeProgress() const;

        bool TryQuickAttack();
        bool TryStartHeavyAttack();
        void ReleaseHeavyAttack();
        void CancelAttack();

    private:
        TransformComponent* transform;
        SpriteMovementAnimationComponent* animation = nullptr;
        MovementComponent* movement = nullptr;
        bool areComponentsSearched = false;

        MeleeAttack quickAttack;
        MeleeAttack heavyAttack;
        float chargeTime = 0.44f;
        std::string targetName;

        const float* lungeSpeeds = nullptr;
        int lungeFramesCount = 0;
        float lungePeakSpeed = 0.f;

        MeleeAttackKind currentKind = MeleeAttackKind::Quick;
        bool isAttacking = false;
        bool isHolding = false;
        bool hasStruck = false;
        float chargeTimer = 0.f;
        float attackTimer = 0.f;
        float cooldownTimer = 0.f;

        std::function<void(MeleeAttackKind)> swingAction;
        std::function<void(MeleeAttackKind, int)> strikeAction;
        std::function<void(MeleeAttackKind, const Vector2Df&, const Vector2Df&)> hitAction;

        const MeleeAttack& GetAttack() const;
        Vector2Df GetForward() const;
        void Start(MeleeAttackKind kind);
        void Strike();
        void Finish();
        void UpdateLunge(float deltaTime);
        void SetMovementEnabled(bool isEnabled);
    };
}
