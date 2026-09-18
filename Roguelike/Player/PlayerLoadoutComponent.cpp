#include "PlayerLoadoutComponent.h"
#include <UiManager.h>
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
        state.Fill(newSlots, newSlotsCount);

        FindComponents();

        int firstArmed = state.FirstArmed(startSlot);
        if (firstArmed != NO_WEAPON_SLOT)
        {
            ApplyWeapon(firstArmed);
        }
    }

    bool PlayerLoadoutComponent::IsSlotEmpty(int slot) const
    {
        return state.IsEmpty(slot);
    }

    bool PlayerLoadoutComponent::CanTakeWeapon(WeaponId id) const
    {
        return state.CanTake(id);
    }

    bool PlayerLoadoutComponent::HasWeapon() const
    {
        return !state.IsEmpty(state.currentSlot);
    }

    bool PlayerLoadoutComponent::EquipWeapon(WeaponId id)
    {
        EquipOutcome equipped = state.Equip(id);
        if (!equipped.isChanged)
        {
            return false;
        }

        if (equipped.slot == state.currentSlot || state.IsEmpty(state.currentSlot))
        {
            ApplyWeapon(equipped.slot);
        }

        LOG_INFO(std::string("Player equips ") + GetWeapon(id).id + " in slot " + std::to_string(equipped.slot + 1));
        return true;
    }

    EquipResult PlayerLoadoutComponent::EquipFromBag(InventoryComponent& bag, int bagSlot, int targetSlot,
        const ItemCatalog& catalog)
    {
        // Живой магазин лежит в оружии, а не в раскладке: без этого обмен печатает патроны.
        SaveCurrentMagazine();

        EquipResult result = TryEquipFromBag(bag, bagSlot, state, targetSlot, catalog);
        if (result.isDone && (result.slot == state.currentSlot || state.IsEmpty(state.currentSlot)))
        {
            ApplyWeapon(result.slot);
        }

        if (result.isDone)
        {
            LOG_INFO(std::string("Player equips ") + GetWeapon(state.slots[result.slot].id).id
                + " in slot " + std::to_string(result.slot + 1));
        }

        return result;
    }

    void PlayerLoadoutComponent::SaveCurrentMagazine()
    {
        if (rangedWeapon == nullptr || IsMeleeEquipped() || state.IsEmpty(state.currentSlot))
        {
            return;
        }

        state.slots[state.currentSlot].magazine = rangedWeapon->GetAmmoInMagazine();
    }

    bool PlayerLoadoutComponent::TrySelectSlot(int slot)
    {
        if (!state.CanSelect(slot) || isSwapping)
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

        SaveCurrentMagazine();

        if (stowedWeapon != nullptr)
        {
            stowedWeapon->SetWeaponId(state.Current());
        }

        pendingSlot = slot;
        isSwapping = true;
        animation->PlaySwap();

        LOG_INFO(std::string("Player swaps to ") + GetWeapon(state.slots[slot].id).id);
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
        return state.Current();
    }

    const LoadoutState& PlayerLoadoutComponent::GetState() const
    {
        return state;
    }

    int PlayerLoadoutComponent::ReadSelectedSlot() const
    {
        // Те же цифры выбирают слот назначения в открытой сумке.
        if (XYZEngine::UiManager::Instance()->IsInputCaptured())
        {
            return NO_WEAPON_SLOT;
        }

        auto input = XYZEngine::InputSystem::Instance();
        for (int slot = 0; slot < state.slotsCount; slot++)
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
        movement = gameObject->GetComponent<XYZEngine::MovementComponent>();
        health = gameObject->GetComponent<HealthComponent>();
    }

    void PlayerLoadoutComponent::ApplyWeapon(int slot)
    {
        state.currentSlot = slot;

        WeaponId id = state.slots[slot].id;
        const WeaponDefinition& definition = GetWeapon(id);

        if (weapon != nullptr)
        {
            weapon->SetWeaponId(id);
        }

        ApplyRangedWeapon(id, state.slots[slot].magazine);
        ApplyMeleeWeapon(FindMelee(id));

        if (movement != nullptr)
        {
            movement->SetSpeed(PLAYER_SPEED * MovePaceOf(id));
        }

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
