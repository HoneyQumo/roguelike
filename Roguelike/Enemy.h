#pragma once

#include "BossCatalog.h"
#include "EnemyConfig.h"
#include <GameObject.h>
#include <Vector.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateEnemy(const EnemyConfig& config, const XYZEngine::Vector2Df& position);
    XYZEngine::GameObject* CreateBoss(const EnemyConfig& config, const BossDefinition& definition, const XYZEngine::Vector2Df& position);
}
