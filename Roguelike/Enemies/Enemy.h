#pragma once

#include "BossCatalog.h"
#include "EnemyConfig.h"
#include "FightStyle.h"
#include <GameObject.h>
#include <Vector.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateEnemy(const EnemyConfig& config, const XYZEngine::Vector2Df& position,
        const BossDefinition* definition = nullptr);
    XYZEngine::GameObject* CreateBoss(const EnemyConfig& config, const BossDefinition& definition, const XYZEngine::Vector2Df& position);

    /**
    *	Выпускает врага сразу по следу игрока: он знает, за кем пришёл, и цель больше не теряет.
    *	Так выходят и свита босса, и волны - те, кого зовут по игрока, а не ставят в караул.
    */
    void SendAfterPlayer(XYZEngine::GameObject* enemy, const XYZEngine::Vector2Df& lastSeen);

    // Тихо проходит мимо тех, у кого погони нет: не всякий созданный объект - враг.
    void ApplyFightStyle(XYZEngine::GameObject* enemy, const FightStyle& style);
}
