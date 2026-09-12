#pragma once

#include <Component.h>
#include <InputComponent.h>
#include "DodgeRollComponent.h"
#include "MeleeWeaponComponent.h"
#include "HealthComponent.h"
#include "PlayerLoadoutComponent.h"
#include "StaminaComponent.h"

namespace RoguelikeGame
{
    class PlayerRollComponent : public XYZEngine::Component
    {
    public:
        PlayerRollComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

    private:
        XYZEngine::InputComponent* input = nullptr;
        DodgeRollComponent* dodgeRoll = nullptr;
        MeleeWeaponComponent* meleeWeapon = nullptr;
        HealthComponent* health = nullptr;
        PlayerLoadoutComponent* loadout = nullptr;
        StaminaComponent* stamina = nullptr;

        bool CanRoll() const;
    };
}
