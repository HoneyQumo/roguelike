#include "PlayerLoadoutComponent.h"
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

        int selectedSlot = input != nullptr ? input->GetSelectedWeaponSlot() : XYZEngine::NO_WEAPON_SLOT;
        if (selectedSlot != XYZEngine::NO_WEAPON_SLOT)
        {
            requestedSlot = selectedSlot;
        }

        if (isSwapping)
        {
            bool isSwapPlaying = animation->GetCurrentAnimation() == XYZEngine::MovementAnimation::Swap;

            if (pendingSlot != XYZEngine::NO_WEAPON_SLOT && (!isSwapPlaying || animation->GetCurrentFrame() >= SWAP_CHANGE_FRAME))
            {
                ApplyWeapon(pendingSlot);
                pendingSlot = XYZEngine::NO_WEAPON_SLOT;
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
        requestedSlot = XYZEngine::NO_WEAPON_SLOT;
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

    void PlayerLoadoutComponent::SetAudio(XYZEngine::AudioComponent* newShotAudio, XYZEngine::AudioComponent* newReloadAudio,
                                          XYZEngine::AudioComponent* newMeleeAudio)
    {
        shotAudio = newShotAudio;
        reloadAudio = newReloadAudio;
        meleeAudio = newMeleeAudio;
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
        ApplyWeapon(std::min(std::max(startSlot, 0), slotsCount - 1));
    }

    bool PlayerLoadoutComponent::TrySelectSlot(int slot)
    {
        if (slot == XYZEngine::NO_WEAPON_SLOT || slot < 0 || slot >= slotsCount || slot == currentSlot || isSwapping)
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

    void PlayerLoadoutComponent::FindComponents()
    {
        if (input == nullptr)
        {
            input = gameObject->GetComponent<XYZEngine::InputComponent>();
        }
        if (animation == nullptr)
        {
            animation = gameObject->GetComponent<XYZEngine::SpriteMovementAnimationComponent>();
        }
        if (rangedWeapon == nullptr)
        {
            rangedWeapon = gameObject->GetComponent<WeaponComponent>();
        }
        if (meleeWeapon == nullptr)
        {
            meleeWeapon = gameObject->GetComponent<MeleeWeaponComponent>();
        }
        if (dodgeRoll == nullptr)
        {
            dodgeRoll = gameObject->GetComponent<DodgeRollComponent>();
        }
        if (health == nullptr)
        {
            health = gameObject->GetComponent<HealthComponent>();
        }
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
