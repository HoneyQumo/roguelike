#pragma once

#include <cmath>
#include <MathUtils.h>
#include <Vector.h>

namespace RoguelikeGame
{
    constexpr float BACKSTAB_HALF_ANGLE = 60.f;

    // hitDirection смотрит от атакующего к цели, поэтому у удара в спину
    // взгляды почти сонаправлены, а при ударе в лицо смотрят врозь.
    inline bool IsBackstab(const XYZEngine::Vector2Df& targetForward, const XYZEngine::Vector2Df& hitDirection,
        float halfAngleDegrees = BACKSTAB_HALF_ANGLE)
    {
        if (targetForward.IsZero() || hitDirection.IsZero() || halfAngleDegrees <= 0.f)
        {
            return false;
        }

        return targetForward.Normalized().DotProduct(hitDirection.Normalized())
            >= std::cos(XYZEngine::ToRadians(halfAngleDegrees));
    }

    inline float BackstabDamage(float damage, bool isBackstab, float critScale)
    {
        return isBackstab && critScale > 1.f ? damage * critScale : damage;
    }
}
