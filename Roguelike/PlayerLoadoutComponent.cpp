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

    void PlayerLoadoutComponent::SetSlots(const WeaponId* newSlots, int newSlotsCount, int startSlot)
    {
        slotsCount = std::min(newSlotsCount, PLAYER_WEAPON_SLOTS);
        for (int slot = 0; slot < slotsCount; slot++)
        {
            slots[slot] = newSlots[slot];
            magazineAmmo[slot] = GetWeapon(newSlots[slot]).magazineSize;
        }

        FindComponents();
        ApplyWeapon(std::clamp(startSlot, 0, slotsCount - 1));
    }

    bool PlayerLoadoutComponent::EquipWeapon(WeaponId id)
    {
        if (slotsCount <= 0)
        {
            return false;
        }

        int slot = IsMelee(id) ? slotsCount - 1 : currentSlot;
        if (IsMelee(slots[slot]) != IsMelee(id))
        {
            for (int index = 0; index < slotsCount; index++)
            {
                if (IsMelee(slots[index]) == IsMelee(id))
                {
                    slot = index;
                    break;
                }
            }
        }

        if (slots[slot] == id)
        {
            return false;
        }

        slots[slot] = id;
        magazineAmmo[slot] = GetWeapon(id).magazineSize;

        if (slot == currentSlot)
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
            magazineAmmo[currentSlot] = rangedWeapon->GetAmmoInMagazine();
        }

        if (stowedWeapon != nullptr)
        {
            stowedWeapon->SetWeaponId(slots[currentSlot]);
        }

        pendingSlot = slot;
        isSwapping = true;
        animation->PlaySwap();

        LOG_INFO(std::string("Player swaps to ") + GetWeapon(slots[slot]).id);
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
        return slots[currentSlot];
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

        WeaponId id = slots[slot];
        const WeaponDefinition& definition = GetWeapon(id);

        if (weapon != nullptr)
        {
            weapon->SetWeaponId(id);
        }

        ApplyRangedWeapon(id, magazineAmmo[slot]);
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
        ShotProfile shot = MakeShotProfile(id, PLAYER_ATTACK_DAMAGE, PLAYER_PROJECTILE_SPEED, PLAYER_ATTACK_COOLDOWN);

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
