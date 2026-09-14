#pragma once

namespace RoguelikeGame
{
    struct LookPlan
    {
        float baseAngle = 0.f;
        float halfSweep = 0.f;
        float duration = 0.f;
    };

    constexpr bool IsLookDone(const LookPlan& plan, float elapsed)
    {
        return plan.duration <= 0.f || elapsed >= plan.duration;
    }

    constexpr float LookAngleAt(const LookPlan& plan, float elapsed)
    {
        if (plan.duration <= 0.f || elapsed <= 0.f)
        {
            return plan.baseAngle;
        }

        if (elapsed >= plan.duration)
        {
            return plan.baseAngle;
        }

        float part = elapsed / plan.duration;
        float toRight = plan.baseAngle + plan.halfSweep;
        float toLeft = plan.baseAngle - plan.halfSweep;

        if (part < 1.f / 3.f)
        {
            return plan.baseAngle + plan.halfSweep * (part * 3.f);
        }

        if (part < 2.f / 3.f)
        {
            return toRight + (toLeft - toRight) * ((part - 1.f / 3.f) * 3.f);
        }

        return toLeft + (plan.baseAngle - toLeft) * ((part - 2.f / 3.f) * 3.f);
    }
}
