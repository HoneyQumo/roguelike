#pragma once

#include "BossCatalog.h"
#include "EnemyConfig.h"
#include <GameObject.h>
#include <Vector.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateEnemy(const EnemyConfig& config, const XYZEngine::Vector2Df& position,
        const BossDefinition* definition = nullptr);
    XYZEngine::GameObject* CreateBoss(const EnemyConfig& config, const BossDefinition& definition, const XYZEngine::Vector2Df& position);
}
