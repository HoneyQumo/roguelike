#pragma once

namespace RoguelikeGame
{
    enum class Faction
    {
        Neutral,
        Player,
        Enemy
    };

    constexpr bool CanDamage(Faction attacker, Faction target)
    {
        return attacker == Faction::Neutral || target == Faction::Neutral || attacker != target;
    }
}
