#include "PlayerLoadoutComponent.h"
#include <InputSystem.h>
#include "WeaponSetup.h"
#include "GameResources.h"
#include "Projectile.h"
#include "Fx.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <algorithm>

namespace RoguelikeGame
{
    PlayerLoadoutComponent::PlayerLoadoutComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
    }

    void PlayerLoadoutComponent::Start()
    {
        FindComponents();
    }

    void PlayerLoadoutComponent::Update(float deltaTime)
    {
        if (animation == nullptr || (health != nullptr && !health->IsAlive()))
        {
            return;
        }

        int selectedSlot = ReadSelectedSlot();
        if (selectedSlot != NO_WEAPON_SLOT)
        {
            requestedSlot = selectedSlot;
        }

        if (isSwapping)
        {
            bool isSwapPlaying = animation->GetCurrentAnimation() == XYZEngine::MovementAnimation::Swap;

            if (pendingSlot != NO_WEAPON_SLOT && (!isSwapPlaying || animation->GetCurrentFrame() >= SWAP_CHANGE_FRAME))
            {
                ApplyWeapon(pendingSlot);
                pendingSlot = NO_WEAPON_SLOT;
            }

            if (!isSwapPlaying || animation->IsFinished())
            {
                isSwapping = false;
            }

            return;
        }

        if (dodgeRoll != nullptr && dodgeRoll->IsRolling())
        {
            return;
        }

        TrySelectSlot(requestedSlot);
        requestedSlot = NO_WEAPON_SLOT;
    }

    void PlayerLoadoutComponent::Render()
    {
    }

    void PlayerLoadoutComponent::SetWeapon(WeaponLayerComponent* newWeapon)
    {
        weapon = newWeapon;
    }

    void PlayerLoadoutComponent::SetStowedWeapon(StowedWeaponComponent* newStowedWeapon)
    {
        stowedWeapon = newStowedWeapon;
    }

    void PlayerLoadoutComponent::SetAudio(XYZEngine::AudioComponent* newShotAudio, XYZEngine::AudioComponent* newReloadAudio)
    {
        shotAudio = newShotAudio;
        reloadAudio = newReloadAudio;
    }

    void PlayerLoadoutComponent::SetSlots(const StartingSlot* newSlots, int newSlotsCount, int startSlot)
    {
        slotsCount = std::min(newSlotsCount, PLAYER_WEAPON_SLOTS);
        for (int slot = 0; slot < slotsCount; slot++)
        {
            slots[slot].id = newSlots[slot].id;
            slots[slot].hasWeapon = newSlots[slot].hasWeapon;
            slots[slot].magazine = newSlots[slot].hasWeapon ? GetWeapon(newSlots[slot].id).magazineSize : 0;
        }

        FindComponents();

        int firstArmed = std::clamp(startSlot, 0, slotsCount - 1);
        if (IsSlotEmpty(firstArmed))
        {
            for (int slot = 0; slot < slotsCount; slot++)
            {
                if (!IsSlotEmpty(slot))
                {
                    firstArmed = slot;
                    break;
                }
            }
        }

        if (!IsSlotEmpty(firstArmed))
        {
            ApplyWeapon(firstArmed);
        }
    }

    bool PlayerLoadoutComponent::IsSlotEmpty(int slot) const
    {
        return slot < 0 || slot >= slotsCount || !slots[slot].hasWeapon;
    }

    bool PlayerLoadoutComponent::CanTakeWeapon(WeaponId id) const
    {
        return slotsCount > 0 && IsSlotEmpty(std::clamp(PreferredWeaponSlot(id), 0, slotsCount - 1));
    }

    bool PlayerLoadoutComponent::HasWeapon() const
    {
        return !IsSlotEmpty(currentSlot);
    }

    bool PlayerLoadoutComponent::EquipWeapon(WeaponId id)
    {
        int slot = std::clamp(PreferredWeaponSlot(id), 0, slotsCount - 1);
        if (slotsCount <= 0 || (slots[slot].hasWeapon && slots[slot].id == id))
        {
            return false;
        }

        slots[slot].id = id;
        slots[slot].hasWeapon = true;
        slots[slot].magazine = GetWeapon(id).magazineSize;

        if (slot == currentSlot || IsSlotEmpty(currentSlot))
        {
            ApplyWeapon(slot);
        }

        LOG_INFO(std::string("Player equips ") + GetWeapon(id).id + " in slot " + std::to_string(slot + 1));
        return true;
    }

    bool PlayerLoadoutComponent::TrySelectSlot(int slot)
    {
        if (slot == NO_WEAPON_SLOT || slot < 0 || slot >= slotsCount || slot == currentSlot || isSwapping)
        {
            return false;
        }

        if (IsSlotEmpty(slot))
        {
            return false;
        }

        if (animation == nullptr || meleeWeapon == nullptr || rangedWeapon == nullptr)
        {
            return false;
        }

        if (meleeWeapon->IsAttacking())
        {
            return false;
        }

        CancelReload();

        if (!IsMeleeEquipped())
        {
            slots[currentSlot].magazine = rangedWeapon->GetAmmoInMagazine();
        }

        if (stowedWeapon != nullptr)
        {
            stowedWeapon->SetWeaponId(slots[currentSlot].id);
        }

        pendingSlot = slot;
        isSwapping = true;
        animation->PlaySwap();

        LOG_INFO(std::string("Player swaps to ") + GetWeapon(slots[slot].id).id);
        return true;
    }

    void PlayerLoadoutComponent::CancelReload()
    {
        if (rangedWeapon != nullptr)
        {
            rangedWeapon->CancelReload();
        }

        if (reloadAudio != nullptr)
        {
            reloadAudio->Stop();
        }
    }

    bool PlayerLoadoutComponent::IsSwapping() const
    {
        return isSwapping;
    }

    bool PlayerLoadoutComponent::IsMeleeEquipped() const
    {
        return IsMelee(GetCurrentWeapon());
    }

    WeaponId PlayerLoadoutComponent::GetCurrentWeapon() const
    {
        return slots[currentSlot].id;
    }

    int PlayerLoadoutComponent::ReadSelectedSlot() const
    {
        auto input = XYZEngine::InputSystem::Instance();
        for (int slot = 0; slot < slotsCount; slot++)
        {
            auto action = static_cast<XYZEngine::InputAction>(static_cast<int>(XYZEngine::InputAction::WeaponSlot1) + slot);
            if (input->WasActionPressed(action))
            {
                return slot;
            }
        }

        return NO_WEAPON_SLOT;
    }

    void PlayerLoadoutComponent::FindComponents()
    {
        animation = gameObject->GetComponent<XYZEngine::SpriteMovementAnimationComponent>();
        rangedWeapon = gameObject->GetComponent<WeaponComponent>();
        meleeWeapon = gameObject->GetComponent<MeleeWeaponComponent>();
        dodgeRoll = gameObject->GetComponent<DodgeRollComponent>();
        health = gameObject->GetComponent<HealthComponent>();
    }

    void PlayerLoadoutComponent::ApplyWeapon(int slot)
    {
        currentSlot = slot;

        WeaponId id = slots[slot].id;
        const WeaponDefinition& definition = GetWeapon(id);

        if (weapon != nullptr)
        {
            weapon->SetWeaponId(id);
        }

        ApplyRangedWeapon(id, slots[slot].magazine);
        ApplyMeleeWeapon(FindMelee(id));

        if (animation != nullptr)
        {
            animation->SetReloadAnimation(PLAYER_TEXTURE, AtlasFrameIndex(RELOAD_ANIMATION.row, 0), RELOAD_ANIMATION.frames,
                                          ReloadFrameSeconds(definition.reloadTime));
        }
    }

    void PlayerLoadoutComponent::ApplyRangedWeapon(WeaponId id, int ammoInMagazine)
    {
        if (rangedWeapon == nullptr)
        {
            return;
        }

        const WeaponDefinition& definition = GetWeapon(id);
        ShotProfile shot = MakeWeaponShotProfile(id, PLAYER_ATTACK_DAMAGE, PLAYER_PROJECTILE_SPEED, PLAYER_ATTACK_COOLDOWN);

        rangedWeapon->CancelReload();
        ApplyWeaponDefinition(rangedWeapon, id, shot);
        rangedWeapon->SetAmmoInMagazine(ammoInMagazine);

        if (shotAudio != nullptr)
        {
            shotAudio->Stop();
            shotAudio->SetSound(GameResources::GetWeaponSound(definition.shotSound));
        }

        if (reloadAudio != nullptr)
        {
            reloadAudio->Stop();
            reloadAudio->SetSound(GameResources::GetWeaponSound(definition.reloadSound));
        }
    }

    void PlayerLoadoutComponent::ApplyMeleeWeapon(const MeleeDefinition* melee)
    {
        if (meleeWeapon == nullptr)
        {
            return;
        }

        meleeWeapon->CancelAttack();
        meleeWeapon->SetDefinition(melee);

        if (melee == nullptr)
        {
            return;
        }

        meleeWeapon->SetQuickAttack(MakeQuickAttack(melee->quick, PLAYER_MELEE_DAMAGE, melee->quick.recovery));
        meleeWeapon->SetHeavyAttack(MakeHeavyAttack(melee->heavy, PLAYER_MELEE_DAMAGE));
        meleeWeapon->SetChargeTime(HEAVY_CHARGE_TIME);
        meleeWeapon->SetLunge(HEAVY_MOVE_SPEED, HEAVY_ANIMATION_FRAMES, PLAYER_HEAVY_LUNGE_SPEED);
    }
}
