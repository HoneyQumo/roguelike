#pragma once

#include <string>
#include "GameSettings.h"
#include "WeaponCatalog.h"
#include <WeaponComponent.h>
#include <MeleeWeaponComponent.h>
#include <AudioComponent.h>
#include <SpriteMovementAnimationComponent.h>

namespace RoguelikeGame
{
    class WeaponLayerComponent;

    inline XYZEngine::MeleeAttack MakeQuickAttack(const MeleeAttackProfile& profile, float baseDamage, float recovery)
    {
        XYZEngine::MeleeAttack attack;
        attack.damage = baseDamage * profile.damageScale;
        attack.chargedDamage = attack.damage;
        attack.range = profile.range;
        attack.arcDegrees = profile.arcDegrees;
        attack.recovery = recovery;
        attack.hitFrame = MELEE_HIT_FRAME;
        attack.windup = MELEE_HIT_FRAME / MELEE_ANIMATION.framesPerSecond;
        return attack;
    }

    inline XYZEngine::MeleeAttack MakeHeavyAttack(const MeleeAttackProfile& profile, float baseDamage)
    {
        XYZEngine::MeleeAttack attack;
        attack.damage = baseDamage * profile.damageScale;
        attack.chargedDamage = baseDamage * profile.chargedDamageScale;
        attack.range = profile.range;
        attack.arcDegrees = profile.arcDegrees;
        attack.recovery = profile.recovery;
        attack.hitFrame = HEAVY_HIT_FIRST_FRAME;
        attack.windup = HEAVY_FRAME_SECONDS[HEAVY_RELEASE_FRAME];
        return attack;
    }

    void ApplyWeaponDefinition(XYZEngine::WeaponComponent* weapon, WeaponId id, const ShotProfile& shot);

    void SpawnProjectilesOnShot(XYZEngine::WeaponComponent* weapon, const std::string& shooterName, WeaponId id);
    void PlayEffectsOnShot(XYZEngine::WeaponComponent* weapon, XYZEngine::AudioComponent* shotAudio,
                          XYZEngine::SpriteMovementAnimationComponent* animation, WeaponLayerComponent* weaponLayer);
    void PlayEffectsOnReload(XYZEngine::WeaponComponent* weapon, XYZEngine::SpriteMovementAnimationComponent* animation,
                            XYZEngine::AudioComponent* reloadAudio);
    void PlayEffectsOnMeleeHit(XYZEngine::MeleeWeaponComponent* melee, XYZEngine::AudioComponent* meleeAudio,
                           const MeleeDefinition* definition);
}
