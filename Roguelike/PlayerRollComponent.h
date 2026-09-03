#pragma once

#include <Component.h>
#include <InputComponent.h>
#include <DodgeRollComponent.h>
#include <MeleeWeaponComponent.h>
#include <HealthComponent.h>
#include "PlayerLoadoutComponent.h"

namespace RoguelikeGame
{
    class PlayerRollComponent : public XYZEngine::Component
    {
    public:
        PlayerRollComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

    private:
        XYZEngine::InputComponent* input = nullptr;
        XYZEngine::DodgeRollComponent* dodgeRoll = nullptr;
        XYZEngine::MeleeWeaponComponent* meleeWeapon = nullptr;
        XYZEngine::HealthComponent* health = nullptr;
        PlayerLoadoutComponent* loadout = nullptr;

        bool wasRollPressed = false;

        bool CanRoll() const;
    };
}
