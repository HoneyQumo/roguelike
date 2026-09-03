#include "WeaponSetup.h"
#include "WeaponLayerComponent.h"
#include "GameResources.h"
#include "Projectile.h"
#include "Fx.h"

namespace RoguelikeGame
{
    void ApplyWeaponDefinition(XYZEngine::WeaponComponent* weapon, WeaponId id, const ShotProfile& shot)
    {
        const WeaponDefinition& definition = GetWeapon(id);

        weapon->SetCooldown(shot.cooldown);
        weapon->SetDamage(shot.damage);
        weapon->SetProjectileSpeed(shot.speed);
        weapon->SetPellets(shot.pellets);
        weapon->SetConeDegrees(shot.coneDegrees);
        weapon->SetMuzzleOffset(ShotOffset(definition));
        weapon->SetMagazine(definition.magazineSize, AmmoKindKey(definition.ammo));
        weapon->SetReloadTime(definition.reloadTime);
    }

    void SpawnProjectilesOnShot(XYZEngine::WeaponComponent* weapon, const std::string& shooterName, WeaponId id)
    {
        weapon->SetShotAction([shooterName, id](const XYZEngine::Vector2Df& shotPosition, const XYZEngine::Vector2Df& shotDirection,
                                                float damage, float speed)
        {
            Projectile::Spawn(shotPosition, shotDirection, damage, speed, shooterName, id);
        });
    }

    void PlayEffectsOnShot(XYZEngine::WeaponComponent* weapon, XYZEngine::AudioComponent* shotAudio,
                          XYZEngine::SpriteMovementAnimationComponent* animation, WeaponLayerComponent* weaponLayer)
    {
        weapon->SetShotStartAction([shotAudio, animation, weaponLayer]()
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

    void PlayEffectsOnReload(XYZEngine::WeaponComponent* weapon, XYZEngine::SpriteMovementAnimationComponent* animation,
                            XYZEngine::AudioComponent* reloadAudio)
    {
        weapon->SetReloadStartAction([animation, reloadAudio]()
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

    void PlayEffectsOnMeleeHit(XYZEngine::MeleeWeaponComponent* melee, XYZEngine::AudioComponent* meleeAudio,
                           const MeleeDefinition* definition)
    {
        melee->SetStrikeAction([meleeAudio, definition](XYZEngine::MeleeAttackKind kind, int hits)
        {
            if (hits <= 0 || meleeAudio == nullptr || definition == nullptr)
            {
                return;
            }

            meleeAudio->SetSound(GameResources::GetMeleeHitSound(*definition));
            meleeAudio->Play();
        });

        melee->SetHitAction([](XYZEngine::MeleeAttackKind kind, const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction)
        {
            Fx::SpawnBloodHit(position, direction);
        });
    }
}
