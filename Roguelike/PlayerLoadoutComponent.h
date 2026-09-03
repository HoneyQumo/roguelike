#pragma once

#include <Component.h>
#include <InputComponent.h>
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
        void SetAudio(XYZEngine::AudioComponent* newShotAudio, XYZEngine::AudioComponent* newReloadAudio, XYZEngine::AudioComponent* newMeleeAudio);
        void SetSlots(const WeaponId* newSlots, int newSlotsCount, int startSlot);

        bool TrySelectSlot(int slot);
        void CancelReload();
        bool IsSwapping() const;
        bool IsMeleeEquipped() const;
        WeaponId GetCurrentWeapon() const;

    private:
        WeaponLayerComponent* weapon = nullptr;
        StowedWeaponComponent* stowedWeapon = nullptr;

        XYZEngine::InputComponent* input = nullptr;
        XYZEngine::SpriteMovementAnimationComponent* animation = nullptr;
        WeaponComponent* rangedWeapon = nullptr;
        MeleeWeaponComponent* meleeWeapon = nullptr;
        DodgeRollComponent* dodgeRoll = nullptr;
        HealthComponent* health = nullptr;

        XYZEngine::AudioComponent* shotAudio = nullptr;
        XYZEngine::AudioComponent* reloadAudio = nullptr;
        XYZEngine::AudioComponent* meleeAudio = nullptr;

        WeaponId slots[PLAYER_WEAPON_SLOTS] = {};
        int magazineAmmo[PLAYER_WEAPON_SLOTS] = {};
        int slotsCount = 0;
        int currentSlot = 0;
        int pendingSlot = XYZEngine::NO_WEAPON_SLOT;
        int requestedSlot = XYZEngine::NO_WEAPON_SLOT;
        bool isSwapping = false;

        void FindComponents();
        void ApplyWeapon(int slot);
        void ApplyRangedWeapon(WeaponId id, int ammoInMagazine);
        void ApplyMeleeWeapon(const MeleeDefinition* melee);
    };
}
