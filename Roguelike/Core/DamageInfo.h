#pragma once

#include <string>
#include <GameObject.h>
#include <Vector.h>
#include "Faction.h"

namespace RoguelikeGame
{
    enum class DamageKind
    {
        Unknown,
        Bullet,
        Melee,
        Explosion
    };

    struct DamageSource
    {
        DamageKind kind = DamageKind::Unknown;
        XYZEngine::GameObjectId attackerId = XYZEngine::NO_GAME_OBJECT;
        std::string attackerName;
        Faction attackerFaction = Faction::Neutral;
        XYZEngine::Vector2Df position = {0.f, 0.f};
        XYZEngine::Vector2Df direction = {0.f, 0.f};
    };

    struct DamageInfo
    {
        float amount = 0.f;
        float rawAmount = 0.f;
        bool isLethal = false;
        DamageSource source;
    };

    struct DeathInfo
    {
        XYZEngine::Vector2Df position = {0.f, 0.f};
        float rotation = 0.f;
        DamageSource source;
    };
}
