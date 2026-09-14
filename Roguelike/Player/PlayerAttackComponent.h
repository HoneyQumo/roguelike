#pragma once

#include <Component.h>
#include <InputComponent.h>
#include "WeaponComponent.h"
#include "MeleeWeaponComponent.h"
#include "DodgeRollComponent.h"
#include "HealthComponent.h"
#include "HitFlashComponent.h"
#include "PlayerLoadoutComponent.h"
#include "StaminaComponent.h"

namespace RoguelikeGame
{
    // Отвечает за аттаку. За направление отвечает AimRotationComponent.
    class PlayerAttackComponent : public XYZEngine::Component
    {
    public:
        PlayerAttackComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

    private:
        XYZEngine::InputComponent* input = nullptr;
        WeaponComponent* weapon = nullptr;
        MeleeWeaponComponent* meleeWeapon = nullptr;
        HealthComponent* health = nullptr;
        DodgeRollComponent* dodgeRoll = nullptr;
        PlayerLoadoutComponent* loadout = nullptr;
        StaminaComponent* stamina = nullptr;
        HitFlashComponent* hitFlash = nullptr;

        float glowTimer = 0.f;

        void UpdateMelee(float deltaTime);
        void UpdateRanged();
        void UpdateChargeGlow(float deltaTime);
    };
}
