#pragma once

namespace RoguelikeGame
{
    enum class ChaseMove
    {
        Hold,
        Approach,
        Investigate
    };

    struct ChaseSense
    {
        float distanceToTarget = 0.f;
        float detectionRadius = 0.f;
        float stopDistance = 0.f;
        float distanceToPoint = 0.f;
        float arriveDistance = 0.f;
        bool isAlerted = false;
        bool isForced = false;
        bool hasPoint = false;
    };

    constexpr bool IsTargetDetected(const ChaseSense& sense)
    {
        if (sense.isForced)
        {
            return true;
        }

        return sense.detectionRadius > 0.f && sense.distanceToTarget <= sense.detectionRadius;
    }

    constexpr bool IsEngaged(const ChaseSense& sense)
    {
        return IsTargetDetected(sense) || sense.isAlerted;
    }

    constexpr ChaseMove ChooseChaseMove(const ChaseSense& sense)
    {
        if (IsTargetDetected(sense))
        {
            return sense.distanceToTarget > sense.stopDistance ? ChaseMove::Approach : ChaseMove::Hold;
        }

        if (sense.isAlerted && sense.hasPoint && sense.distanceToPoint > sense.arriveDistance)
        {
            return ChaseMove::Investigate;
        }

        return ChaseMove::Hold;
    }
}
