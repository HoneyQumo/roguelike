#pragma once

#include "BossAnimationComponent.h"
#include "BossBrainComponent.h"
#include <ParticleAuraComponent.h>
#include "HealthBarComponent.h"
#include "HealthComponent.h"
#include "HitFlashComponent.h"
#include "LevelData.h"
#include "WeaponCatalog.h"
#include <GameObject.h>

namespace RoguelikeGame
{
    constexpr TileType BOSS_MINION_TILE = TileType::MarauderSpawn;

    void SpawnProjectilesOnBossVolley(BossBrainComponent* brain, XYZEngine::GameObject* boss, WeaponId weapon);
    void SummonMinionsOnBossCall(BossBrainComponent* brain);
    void PlayEffectsOnBossBlast(BossBrainComponent* brain);
    void PlayEffectsOnBossRage(BossBrainComponent* brain, HitFlashComponent* hitFlash, HealthBarComponent* healthBar,
        XYZEngine::ParticleAuraComponent* rageAura);
    void ShowMarkOnBossCast(BossBrainComponent* brain);
    void PlayBossAnimations(BossBrainComponent* brain, BossAnimationComponent* animation, HealthComponent* health);
}
