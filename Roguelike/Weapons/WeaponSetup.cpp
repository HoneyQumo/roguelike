#include "WeaponSetup.h"
#include "WeaponLayerComponent.h"
#include "GameResources.h"
#include "Projectile.h"
#include "Fx.h"
#include "FactionComponent.h"
#include "Noise.h"

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
    void SpawnProjectilesOnShot(WeaponComponent* weapon)
    {
        weapon->SubscribeShot([weapon](const XYZEngine::Vector2Df& shotPosition, const XYZEngine::Vector2Df& shotDirection,
                                       float damage, float speed)
        {
            Projectile::Spawn(shotPosition, shotDirection, damage, speed, weapon->GetGameObject(), weapon->GetWeaponId());
        });
    }

    void RaiseNoiseOnShot(WeaponComponent* weapon)
    {
        weapon->SubscribeShot([weapon](const XYZEngine::Vector2Df& shotPosition, const XYZEngine::Vector2Df&, float, float)
        {
            Noise noise;
            noise.position = shotPosition;
            noise.radius = GetWeapon(weapon->GetWeaponId()).noiseRadius;
            noise.from = GetFactionOf(weapon->GetGameObject());

            RaiseNoise(noise);
        });
    }

    void PlayEffectsOnShot(WeaponComponent* weapon, SoundPlace place,
                          XYZEngine::SpriteMovementAnimationComponent* animation, WeaponLayerComponent* weaponLayer)
    {
        weapon->SubscribeShotStart([weapon, place, animation, weaponLayer]()
        {
            // Ствол берём в момент выстрела: иначе звук пришлось бы менять при
            // каждой смене оружия и следить, чтобы он не отстал.
            PlayOneShot(GameResources::GetWeaponSound(GetWeapon(weapon->GetWeaponId()).shotSound),
                SHOT_VOLUME, SoundKind::Shot, place, weapon->GetGameObject());

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

    void PlayEffectsOnMeleeHit(MeleeWeaponComponent* melee, SoundPlace place)
    {
        melee->SubscribeStrike([melee, place](MeleeAttackKind kind, int hits, bool isCritical)
        {
            const MeleeDefinition* definition = melee->GetDefinition();
            if (hits <= 0 || definition == nullptr)
            {
                return;
            }

            PlayOneShot(GameResources::GetMeleeHitSound(*definition), MELEE_HIT_VOLUME, SoundKind::Hit, place,
                melee->GetGameObject());
        });


    }
}
