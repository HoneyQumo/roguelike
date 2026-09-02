#include "PlayerLoadoutComponent.h"
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

    void PlayerLoadoutComponent::Update(float deltaTime)
    {
        FindComponents();

        if (animation == nullptr)
        {
            return;
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

        if (input != nullptr && (dodgeRoll == nullptr || !dodgeRoll->IsRolling()))
        {
            TrySelectSlot(input->GetSelectedWeaponSlot());
        }
    }

    void PlayerLoadoutComponent::Render()
    {
    }

    void PlayerLoadoutComponent::SetWeapon(Weapon* newWeapon)
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
            rangedWeapon = gameObject->GetComponent<XYZEngine::WeaponComponent>();
        }
        if (meleeWeapon == nullptr)
        {
            meleeWeapon = gameObject->GetComponent<XYZEngine::MeleeWeaponComponent>();
        }
        if (dodgeRoll == nullptr)
        {
            dodgeRoll = gameObject->GetComponent<XYZEngine::DodgeRollComponent>();
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

        ApplyRangedWeapon(definition, magazineAmmo[slot]);
        ApplyMeleeWeapon(FindMelee(id));

        if (animation != nullptr)
        {
            animation->SetReloadAnimation(PLAYER_TEXTURE, AtlasFrameIndex(RELOAD_ANIMATION.row, 0), RELOAD_ANIMATION.frames,
                                          ReloadFramesPerSecond(definition.reloadTime));
        }
    }

    void PlayerLoadoutComponent::ApplyRangedWeapon(const WeaponDefinition& definition, int ammoInMagazine)
    {
        if (rangedWeapon == nullptr)
        {
            return;
        }

        rangedWeapon->CancelReload();
        rangedWeapon->SetCooldown(PLAYER_ATTACK_COOLDOWN);
        rangedWeapon->SetDamage(PLAYER_ATTACK_DAMAGE);
        rangedWeapon->SetProjectileSpeed(PLAYER_PROJECTILE_SPEED);
        rangedWeapon->SetMuzzleOffset(ShotOffset(definition));
        rangedWeapon->SetMagazine(definition.magazineSize, AmmoKindKey(definition.ammo));
        rangedWeapon->SetAmmoInMagazine(ammoInMagazine);
        rangedWeapon->SetReloadTime(definition.reloadTime);

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

        BulletKind bullet = definition.bullet;
        std::string shooterName = gameObject->GetName();

        rangedWeapon->SetShotAction([this, bullet, shooterName](const XYZEngine::Vector2Df& shotPosition, const XYZEngine::Vector2Df& shotDirection,
                                                                 float damage, float speed)
        {
            Projectile::Spawn(shotPosition, shotDirection, damage, speed, shooterName, bullet);

            if (shotAudio != nullptr)
            {
                shotAudio->Play();
            }

            if (animation != nullptr)
            {
                animation->PlayShoot();
            }

            if (weapon != nullptr)
            {
                weapon->PlayMuzzleFlash();
            }
        });
    }

    void PlayerLoadoutComponent::ApplyMeleeWeapon(const MeleeDefinition* melee)
    {
        if (meleeWeapon == nullptr)
        {
            return;
        }

        meleeWeapon->CancelAttack();

        if (melee == nullptr)
        {
            return;
        }

        XYZEngine::MeleeAttack quick;
        quick.damage = PLAYER_MELEE_DAMAGE * melee->quick.damageScale;
        quick.chargedDamage = quick.damage;
        quick.range = melee->quick.range;
        quick.arcDegrees = melee->quick.arcDegrees;
        quick.recovery = melee->quick.recovery;
        quick.hitFrame = MELEE_HIT_FRAME;
        quick.windup = MELEE_HIT_FRAME / MELEE_ANIMATION.framesPerSecond;

        XYZEngine::MeleeAttack heavy;
        heavy.damage = PLAYER_MELEE_DAMAGE * melee->heavy.damageScale;
        heavy.chargedDamage = PLAYER_MELEE_DAMAGE * melee->heavy.chargedDamageScale;
        heavy.range = melee->heavy.range;
        heavy.arcDegrees = melee->heavy.arcDegrees;
        heavy.recovery = melee->heavy.recovery;
        heavy.hitFrame = HEAVY_HIT_FIRST_FRAME;
        heavy.windup = HEAVY_FRAME_SECONDS[HEAVY_RELEASE_FRAME];

        meleeWeapon->SetQuickAttack(quick);
        meleeWeapon->SetHeavyAttack(heavy);
        meleeWeapon->SetChargeTime(HEAVY_CHARGE_TIME);
        meleeWeapon->SetLunge(HEAVY_MOVE_SPEED, HEAVY_ANIMATION_FRAMES, PLAYER_HEAVY_LUNGE_SPEED);

        const MeleeDefinition* current = melee;
        meleeWeapon->SetStrikeAction([this, current](XYZEngine::MeleeAttackKind kind, int hits)
        {
            if (hits <= 0 || meleeAudio == nullptr)
            {
                return;
            }

            meleeAudio->SetSound(GameResources::GetMeleeHitSound(*current));
            meleeAudio->Play();
        });

        meleeWeapon->SetHitAction([](XYZEngine::MeleeAttackKind kind, const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction)
        {
            Fx::SpawnBloodHit(position, direction);
        });
    }
}
