#pragma once

#include "EnemyConfig.h"
#include <GameObject.h>
#include <Vector.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateEnemy(const EnemyConfig& config, const XYZEngine::Vector2Df& position);
}
