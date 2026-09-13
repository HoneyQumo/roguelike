#pragma once

#include <Component.h>
#include <AudioComponent.h>
#include "WeaponComponent.h"
#include "MeleeWeaponComponent.h"
#include "DodgeRollComponent.h"
#include "HealthComponent.h"
#include <SpriteMovementAnimationComponent.h>
#include "GameSettings.h"
#include "Weapon.h"
#include "StowedWeaponComponent.h"

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
        void SetAudio(XYZEngine::AudioComponent* newShotAudio, XYZEngine::AudioComponent* newReloadAudio);
        void SetSlots(const StartingSlot* newSlots, int newSlotsCount, int startSlot);

        bool TrySelectSlot(int slot);
        bool EquipWeapon(WeaponId id);
        bool IsSlotEmpty(int slot) const;
        bool HasWeapon() const;
        void CancelReload();
        bool IsSwapping() const;
        bool IsMeleeEquipped() const;
        WeaponId GetCurrentWeapon() const;

    private:
        WeaponLayerComponent* weapon = nullptr;
        StowedWeaponComponent* stowedWeapon = nullptr;

        XYZEngine::SpriteMovementAnimationComponent* animation = nullptr;
        WeaponComponent* rangedWeapon = nullptr;
        MeleeWeaponComponent* meleeWeapon = nullptr;
        DodgeRollComponent* dodgeRoll = nullptr;
        HealthComponent* health = nullptr;

        XYZEngine::AudioComponent* shotAudio = nullptr;
        XYZEngine::AudioComponent* reloadAudio = nullptr;

        struct Slot
        {
            WeaponId id = WeaponId::Knife;
            int magazine = 0;
            bool hasWeapon = false;
        };

        Slot slots[PLAYER_WEAPON_SLOTS] = {};
        int slotsCount = 0;
        int currentSlot = 0;
        int pendingSlot = NO_WEAPON_SLOT;
        int requestedSlot = NO_WEAPON_SLOT;
        bool isSwapping = false;

        int ReadSelectedSlot() const;
        void FindComponents();
        void ApplyWeapon(int slot);
        void ApplyRangedWeapon(WeaponId id, int ammoInMagazine);
        void ApplyMeleeWeapon(const MeleeDefinition* melee);
    };
}
