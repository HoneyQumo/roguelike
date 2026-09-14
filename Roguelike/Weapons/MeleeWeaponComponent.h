#pragma once

#include <functional>
#include <string>
#include <Component.h>
#include <EventList.h>
#include "FactionComponent.h"
#include "WeaponCatalog.h"
#include <TransformComponent.h>
#include <MovementComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include <Vector.h>
#include <Cooldown.h>

namespace RoguelikeGame
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

    class MeleeWeaponComponent : public XYZEngine::Component
    {
    public:
        MeleeWeaponComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetQuickAttack(const MeleeAttack& newQuickAttack);
        void SetHeavyAttack(const MeleeAttack& newHeavyAttack);
        void SetChargeTime(float newChargeTime);
        void SetLunge(const float* frameSpeeds, int framesCount, float peakSpeed);

        void SetDefinition(const MeleeDefinition* newDefinition);
        const MeleeDefinition* GetDefinition() const;

        XYZEngine::SubscriptionId SubscribeSwing(std::function<void(MeleeAttackKind)> onSwing);
        XYZEngine::SubscriptionId SubscribeStrike(std::function<void(MeleeAttackKind, int)> onStrike);
        XYZEngine::SubscriptionId SubscribeHit(std::function<void(MeleeAttackKind, const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&)> onHit);

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
        XYZEngine::TransformComponent* transform = nullptr;
        XYZEngine::SpriteMovementAnimationComponent* animation = nullptr;
        XYZEngine::MovementComponent* movement = nullptr;

        MeleeAttack quickAttack;
        MeleeAttack heavyAttack;
        float chargeTime = 0.44f;
        FactionComponent* faction = nullptr;

        const float* lungeSpeeds = nullptr;
        int lungeFramesCount = 0;
        float lungePeakSpeed = 0.f;

        MeleeAttackKind currentKind = MeleeAttackKind::Quick;
        bool isAttacking = false;
        bool isHolding = false;
        bool hasStruck = false;
        float chargeTimer = 0.f;
        float attackTimer = 0.f;
        XYZEngine::Cooldown cooldown;

        const MeleeDefinition* definition = nullptr;

        XYZEngine::EventList<MeleeAttackKind> swingEvent;
        XYZEngine::EventList<MeleeAttackKind, int> strikeEvent;
        XYZEngine::EventList<MeleeAttackKind, const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&> hitEvent;

        const MeleeAttack& GetAttack() const;
        XYZEngine::Vector2Df GetForward() const;
        void BeginAttack(MeleeAttackKind kind);
        void Strike();
        void Finish();
        void UpdateLunge(float deltaTime);
        void SetMovementEnabled(bool isEnabled);
    };
}
