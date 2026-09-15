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


    constexpr float VISION_PERIPHERY_PART = 0.6f;
    constexpr float VISION_BACK_DISTANCE = 96.f;

    enum class VisionBand
    {
        None,
        Focus,
        Periphery,
        Back
    };

    struct VisionField
    {
        float maxDistance = 0.f;
        float focusHalfAngle = 60.f;
        float peripheryHalfAngle = 105.f;
        float peripheryPart = VISION_PERIPHERY_PART;
        float backDistance = VISION_BACK_DISTANCE;
    };

    inline float AngleBetweenDegrees(const XYZEngine::Vector2Df& facing, const XYZEngine::Vector2Df& toTarget)
    {
        float facingLength = facing.GetLength();
        float targetLength = toTarget.GetLength();

        if (facingLength <= 0.f || targetLength <= 0.f)
        {
            return 0.f;
        }

        float cosine = (facing.x * toTarget.x + facing.y * toTarget.y) / (facingLength * targetLength);
        cosine = cosine > 1.f ? 1.f : (cosine < -1.f ? -1.f : cosine);

        return XYZEngine::ToDegrees(std::acos(cosine));
    }

    inline float BandDistance(const VisionField& field, VisionBand band)
    {
        if (band == VisionBand::Periphery)
        {
            return field.maxDistance * field.peripheryPart;
        }

        return band == VisionBand::Back ? field.backDistance : field.maxDistance;
    }

    inline VisionBand BandFor(const VisionField& field, const XYZEngine::Vector2Df& facing,
        const XYZEngine::Vector2Df& toTarget, bool isWallBetween)
    {
        if (field.maxDistance <= 0.f || isWallBetween)
        {
            return VisionBand::None;
        }

        float distance = toTarget.GetLength();
        if (distance > field.maxDistance)
        {
            return VisionBand::None;
        }

        float angle = AngleBetweenDegrees(facing, toTarget);

        if (angle <= field.focusHalfAngle)
        {
            return VisionBand::Focus;
        }

        if (angle <= field.peripheryHalfAngle && distance <= BandDistance(field, VisionBand::Periphery))
        {
            return VisionBand::Periphery;
        }

        return distance <= field.backDistance ? VisionBand::Back : VisionBand::None;
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
