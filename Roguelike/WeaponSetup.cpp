#include "WeaponSetup.h"
#include "WeaponLayerComponent.h"
#include "GameResources.h"
#include "Projectile.h"
#include "Fx.h"

namespace RoguelikeGame
{
    void ApplyWeaponDefinition(WeaponComponent* weapon, WeaponId id, const ShotProfile& shot)
    {
        const WeaponDefinition& definition = GetWeapon(id);

        weapon->SetCooldown(shot.cooldown);
        weapon->SetDamage(shot.damage);
        weapon->SetProjectileSpeed(shot.speed);
        weapon->SetPellets(shot.pellets);
        weapon->SetConeDegrees(shot.coneDegrees);
        weapon->SetMuzzleOffset(ShotOffset(definition));
        weapon->SetWeaponId(id);
        weapon->SetMagazine(definition.magazineSize, AmmoKindKey(definition.ammo));
        weapon->SetReloadTime(definition.reloadTime);
    }

    // Ствол меняется на ходу, поэтому идентификатор берётся у компонента, а не запоминается подпиской.
    void SpawnProjectilesOnShot(WeaponComponent* weapon, const std::string& shooterName)
    {
        weapon->SubscribeShot([weapon, shooterName](const XYZEngine::Vector2Df& shotPosition, const XYZEngine::Vector2Df& shotDirection,
                                                    float damage, float speed)
        {
            Projectile::Spawn(shotPosition, shotDirection, damage, speed, shooterName, weapon->GetWeaponId());
        });
    }

    void PlayEffectsOnShot(WeaponComponent* weapon, XYZEngine::AudioComponent* shotAudio,
                          XYZEngine::SpriteMovementAnimationComponent* animation, WeaponLayerComponent* weaponLayer)
    {
        weapon->SubscribeShotStart([shotAudio, animation, weaponLayer]()
        {
            if (shotAudio != nullptr)
            {
                shotAudio->Play();
            }

            if (animation != nullptr)
            {
                animation->PlayShoot();
            }

            if (weaponLayer != nullptr)
            {
                weaponLayer->PlayMuzzleFlash();
            }
        });
    }

    void PlayEffectsOnReload(WeaponComponent* weapon, XYZEngine::SpriteMovementAnimationComponent* animation,
                            XYZEngine::AudioComponent* reloadAudio)
    {
        weapon->SubscribeReloadStart([animation, reloadAudio]()
        {
            if (animation != nullptr)
            {
                animation->PlayReload();
            }

            if (reloadAudio != nullptr)
            {
                reloadAudio->Play();
            }
        });
    }

    void PlayEffectsOnMeleeHit(MeleeWeaponComponent* melee, XYZEngine::AudioComponent* meleeAudio)
    {
        melee->SubscribeStrike([melee, meleeAudio](MeleeAttackKind kind, int hits)
        {
            const MeleeDefinition* definition = melee->GetDefinition();
            if (hits <= 0 || meleeAudio == nullptr || definition == nullptr)
            {
                return;
            }

            meleeAudio->SetSound(GameResources::GetMeleeHitSound(*definition));
            meleeAudio->Play();
        });

        melee->SubscribeHit([](MeleeAttackKind kind, const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction)
        {
            Fx::SpawnBloodHit(position, direction);
        });
    }
}
