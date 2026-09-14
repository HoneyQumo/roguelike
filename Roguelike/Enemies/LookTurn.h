#pragma once

#include "LookAround.h"

namespace RoguelikeGame
{
    class LookTurn
    {
    public:
        void Start(float baseAngle, float halfSweep, float duration)
        {
            plan.baseAngle = baseAngle;
            plan.halfSweep = halfSweep;
            plan.duration = duration;
            elapsed = 0.f;
            isRunning = duration > 0.f;
        }

        void Stop()
        {
            isRunning = false;
        }

        void Tick(float deltaTime)
        {
            if (isRunning)
            {
                elapsed += deltaTime;
            }
        }

        bool IsRunning() const
        {
            return isRunning;
        }

        bool IsDone() const
        {
            return !isRunning || IsLookDone(plan, elapsed);
        }

        float GetAngle() const
        {
            return LookAngleAt(plan, elapsed);
        }

    private:
        LookPlan plan;
        float elapsed = 0.f;
        bool isRunning = false;
    };
}
