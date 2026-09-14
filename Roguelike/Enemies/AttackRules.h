#pragma once

namespace RoguelikeGame
{
    struct AttackSense
    {
        float distanceToTarget = 0.f;
        float attackRange = 0.f;
        bool isAlive = false;
        bool hasTarget = false;
        bool isTargetAlive = false;
        bool canSeeTarget = false;
    };

    constexpr bool MayAttack(const AttackSense& sense)
    {
        if (!sense.isAlive || !sense.hasTarget || !sense.isTargetAlive)
        {
            return false;
        }

        if (!sense.canSeeTarget || sense.attackRange <= 0.f)
        {
            return false;
        }

        return sense.distanceToTarget <= sense.attackRange;
    }
}
