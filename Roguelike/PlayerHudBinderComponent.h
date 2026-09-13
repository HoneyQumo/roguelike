#pragma once

#include <string>
#include <Component.h>
#include "HudScreen.h"
#include "HealthComponent.h"
#include "InteractionComponent.h"
#include "InventoryComponent.h"
#include "PlayerLoadoutComponent.h"
#include "StaminaComponent.h"
#include "WeaponComponent.h"

namespace RoguelikeGame
{
    class PlayerHudBinderComponent : public XYZEngine::Component
    {
    public:
        PlayerHudBinderComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetTargetName(const std::string& newTargetName);
        void SetScreen(HudScreen* newScreen);

    private:
        HudScreen* screen = nullptr;
        WeaponComponent* weapon = nullptr;
        PlayerLoadoutComponent* loadout = nullptr;
        HealthComponent* health = nullptr;
        StaminaComponent* stamina = nullptr;
        InventoryComponent* inventory = nullptr;
        InteractionComponent* interaction = nullptr;

        std::string targetName;

        void FindTarget();
        AmmoHudState ReadAmmoState() const;
        VitalsHudState ReadVitalsState() const;
    };
}
