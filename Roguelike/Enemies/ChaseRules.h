#pragma once

namespace RoguelikeGame
{
    enum class ChaseMove
    {
        Hold,
        Approach,
        Investigate,
        Withdraw,
        TakeCover
    };

    struct ChaseSense
    {
        float distanceToTarget = 0.f;
        float detectionRadius = 0.f;
        float stopDistance = 0.f;
        float backOffDistance = 0.f;
        float distanceToPoint = 0.f;
        float arriveDistance = 0.f;
        bool isAlerted = false;
        bool isForced = false;
        bool isReloading = false;
        bool isLowOnAmmo = false;
        bool hasCover = false;
        bool hasPoint = false;
        bool isVisible = false;
    };

    constexpr bool IsTargetDetected(const ChaseSense& sense)
    {
        if (sense.isForced)
        {
            return true;
        }

        return sense.isVisible;
    }

    constexpr bool IsEngaged(const ChaseSense& sense)
    {
        return IsTargetDetected(sense) || sense.isAlerted;
    }

    constexpr ChaseMove ChooseChaseMove(const ChaseSense& sense)
    {
        // До проверки видимости: из укрытия игрока не видно, и враг бросился бы
        // его искать, не дозарядив. Стиль боя гасится выше, в ReadSense.
        if ((sense.isReloading || sense.isLowOnAmmo) && sense.hasCover)
        {
            return ChaseMove::TakeCover;
        }

        if (IsTargetDetected(sense))
        {
            if (sense.distanceToTarget > sense.stopDistance)
            {
                return ChaseMove::Approach;
            }

            // Между отходом и остановкой мёртвая зона, иначе враг дёргается на границе.
            // У ножевика она шире его же stopDistance, и отход недостижим.
            return sense.distanceToTarget < sense.backOffDistance ? ChaseMove::Withdraw : ChaseMove::Hold;
        }

        if (sense.isAlerted && sense.hasPoint && sense.distanceToPoint > sense.arriveDistance)
        {
            return ChaseMove::Investigate;
        }

        return ChaseMove::Hold;
    }
}
