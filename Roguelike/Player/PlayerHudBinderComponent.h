#pragma once

#include <string>
#include <Component.h>
#include "HudScreen.h"
#include "AmmoPouchComponent.h"
#include "HealthComponent.h"
#include "InteractionComponent.h"
#include "InventoryComponent.h"
#include "QuickBeltComponent.h"
#include "ItemEffectComponent.h"
#include "InventoryScreen.h"
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
        void SetInventoryScreen(InventoryScreen* newInventoryScreen);

    private:
        HudScreen* screen = nullptr;
        InventoryScreen* inventoryScreen = nullptr;
        AmmoPouchComponent* pouch = nullptr;
        QuickBeltComponent* belt = nullptr;

        void ShowRefusal(const std::string& text);
        void PushWeaponSlots();
        void PushBeltSlots();
        void FillAmmo(SlotHudState& shown, WeaponId id, bool isCurrent, int remembered) const;
        WeaponComponent* weapon = nullptr;
        PlayerLoadoutComponent* loadout = nullptr;
        HealthComponent* health = nullptr;
        StaminaComponent* stamina = nullptr;
        InventoryComponent* inventory = nullptr;
        InteractionComponent* interaction = nullptr;

        std::string targetName;

        void FindTarget();

        VitalsHudState ReadVitalsState() const;
    };
}
