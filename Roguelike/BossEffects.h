#pragma once

#include "BossBrainComponent.h"
#include <ParticleEmitterComponent.h>
#include "HealthBarComponent.h"
#include "HitFlashComponent.h"
#include "LevelData.h"
#include "WeaponCatalog.h"
#include <GameObject.h>

namespace RoguelikeGame
{
    constexpr TileType BOSS_MINION_TILE = TileType::GruntSpawn;

    void SpawnProjectilesOnBossVolley(BossBrainComponent* brain, XYZEngine::GameObject* boss, WeaponId weapon);
    void SummonMinionsOnBossCall(BossBrainComponent* brain);
    void PlayEffectsOnBossBlast(BossBrainComponent* brain);
    void PlayEffectsOnBossRage(BossBrainComponent* brain, HitFlashComponent* hitFlash, HealthBarComponent* healthBar,
        XYZEngine::ParticleEmitterComponent* rageAura);
    void ShowMarkOnBossCast(BossBrainComponent* brain);
}
