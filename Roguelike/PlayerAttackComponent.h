#pragma once

#include <Component.h>
#include <InputComponent.h>
#include <WeaponComponent.h>
#include <MeleeWeaponComponent.h>
#include <DodgeRollComponent.h>
#include <HealthComponent.h>
#include "HitFlashComponent.h"
#include "PlayerLoadoutComponent.h"

namespace RoguelikeGame
{
    // Отвечает за аттаку. За направление отвечает AimRotationComponent.
    class PlayerAttackComponent : public XYZEngine::Component
    {
    public:
        PlayerAttackComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

    private:
        XYZEngine::InputComponent* input = nullptr;
        XYZEngine::WeaponComponent* weapon = nullptr;
        XYZEngine::MeleeWeaponComponent* meleeWeapon = nullptr;
        XYZEngine::HealthComponent* health = nullptr;
        XYZEngine::DodgeRollComponent* dodgeRoll = nullptr;
        PlayerLoadoutComponent* loadout = nullptr;
        HitFlashComponent* hitFlash = nullptr;

        float glowTimer = 0.f;

        void UpdateMelee(float deltaTime);
        void UpdateRanged();
        void UpdateChargeGlow(float deltaTime);
    };
}
