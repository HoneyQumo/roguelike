#pragma once

#include <MathUtils.h>
#include <Vector.h>
#include <algorithm>

namespace RoguelikeGame
{
    struct VisionCone
    {
        float maxDistance = 0.f;
        float halfAngleDegrees = 180.f;
    };

    struct VisionRange
    {
        float maxDistance = 0.f;
        float calmHalfAngle = 180.f;
        float alertHalfAngle = 180.f;
    };

    inline VisionCone ConeFor(const VisionRange& range, bool isAlerted)
    {
        VisionCone cone;
        cone.maxDistance = range.maxDistance;
        cone.halfAngleDegrees = isAlerted ? std::max(range.calmHalfAngle, range.alertHalfAngle) : range.calmHalfAngle;

        return cone;
    }

    inline bool IsWithinCone(const XYZEngine::Vector2Df& facing, const XYZEngine::Vector2Df& toTarget, float halfAngleDegrees)
    {
        if (halfAngleDegrees >= 180.f)
        {
            return true;
        }

        if (halfAngleDegrees <= 0.f)
        {
            return false;
        }

        float facingLength = facing.GetLength();
        float targetLength = toTarget.GetLength();

        if (facingLength <= 0.f || targetLength <= 0.f)
        {
            return true;
        }

        float cosine = (facing.x * toTarget.x + facing.y * toTarget.y) / (facingLength * targetLength);
        cosine = cosine > 1.f ? 1.f : (cosine < -1.f ? -1.f : cosine);

        return cosine >= std::cos(XYZEngine::ToRadians(halfAngleDegrees));
    }

    inline bool CanSeeTarget(const VisionCone& cone, const XYZEngine::Vector2Df& facing,
        const XYZEngine::Vector2Df& toTarget, bool isWallBetween)
    {
        if (cone.maxDistance <= 0.f || isWallBetween)
        {
            return false;
        }

        if (toTarget.GetLength() > cone.maxDistance)
        {
            return false;
        }

        return IsWithinCone(facing, toTarget, cone.halfAngleDegrees);
    }
}
