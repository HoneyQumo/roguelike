#pragma once

#include <cmath>
#include <Vector.h>

namespace RoguelikeGame
{
    // Доля пути считается от времени кадра, иначе на разной частоте камера едет по-разному.
    inline XYZEngine::Vector2Df ApproachPoint(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to,
        float smoothTime, float deltaTime)
    {
        if (smoothTime <= 0.f || deltaTime <= 0.f)
        {
            return to;
        }

        return from + (to - from) * (1.f - std::exp(-deltaTime / smoothTime));
    }

    inline float ClampAxis(float center, float halfView, float min, float max)
    {
        // Карта уже кадра - зажимать нечем, обе границы тянут в разные стороны.
        if (max - min <= 2.f * halfView)
        {
            return 0.5f * (min + max);
        }

        float low = min + halfView;
        float high = max - halfView;

        return center < low ? low : (center > high ? high : center);
    }

    inline XYZEngine::Vector2Df ClampToBounds(const XYZEngine::Vector2Df& center, const XYZEngine::Vector2Df& halfView,
        const XYZEngine::Vector2Df& min, const XYZEngine::Vector2Df& max)
    {
        return {ClampAxis(center.x, halfView.x, min.x, max.x), ClampAxis(center.y, halfView.y, min.y, max.y)};
    }
}
