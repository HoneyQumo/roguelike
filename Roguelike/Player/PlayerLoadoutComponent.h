#pragma once

#include <Component.h>
#include <AudioComponent.h>
#include "WeaponComponent.h"
#include "MeleeWeaponComponent.h"
#include "DodgeRollComponent.h"
#include "HealthComponent.h"
#include <MovementComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include "GameSettings.h"
#include "Weapon.h"
#include "StowedWeaponComponent.h"
#include "LoadoutRules.h"
#include "EquipExchange.h"

namespace RoguelikeGame
{
    /**
    *	Три слота оружия: основное, второстепенное и ближний бой.
    */
    class PlayerLoadoutComponent : public XYZEngine::Component
    {
    public:
        PlayerLoadoutComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetWeapon(WeaponLayerComponent* newWeapon);
        void SetStowedWeapon(StowedWeaponComponent* newStowedWeapon);
        void SetAudio(XYZEngine::AudioComponent* newReloadAudio);
        void SetSlots(const StartingSlot* newSlots, int newSlotsCount, int startSlot);

        bool TrySelectSlot(int slot);
        bool EquipWeapon(WeaponId id);
        EquipResult EquipFromBag(InventoryComponent& bag, int bagSlot, int targetSlot, const ItemCatalog& catalog);
        bool IsSlotEmpty(int slot) const;
        bool CanTakeWeapon(WeaponId id) const;
        bool HasWeapon() const;
        void CancelReload();
        bool IsSwapping() const;
        bool IsMeleeEquipped() const;
        WeaponId GetCurrentWeapon() const;

        // Раскладку читают снаружи: HUD должен показывать все слоты, а не только текущий.
        const LoadoutState& GetState() const;

    private:
        WeaponLayerComponent* weapon = nullptr;
        StowedWeaponComponent* stowedWeapon = nullptr;

        XYZEngine::SpriteMovementAnimationComponent* animation = nullptr;
        WeaponComponent* rangedWeapon = nullptr;
        MeleeWeaponComponent* meleeWeapon = nullptr;
        DodgeRollComponent* dodgeRoll = nullptr;
        XYZEngine::MovementComponent* movement = nullptr;
        HealthComponent* health = nullptr;

        XYZEngine::AudioComponent* reloadAudio = nullptr;

        LoadoutState state;
        int pendingSlot = NO_WEAPON_SLOT;
        int requestedSlot = NO_WEAPON_SLOT;
        bool isSwapping = false;

        int ReadSelectedSlot() const;
        void SaveCurrentMagazine();
        void FindComponents();
        void ApplyWeapon(int slot);
        void ApplyRangedWeapon(WeaponId id, int ammoInMagazine);
        void ApplyMeleeWeapon(const MeleeDefinition* melee);
    };
}
