#pragma once

#include <cmath>
#include <Vector.h>

namespace RoguelikeGame
{
    /**
    *	Как камера догоняет героя и почему не выезжает за карту.
    *
    *	Оба правила чистые: в них нет ни объекта, ни сетки уровня, поэтому их
    *	можно проверить числами, а не глазами.
    */

    /**
    *	Мягкое доведение к точке.
    *
    *	Доля пути считается через экспоненту, а не берётся фиксированной: иначе
    *	на разной частоте кадров камера ехала бы с разной скоростью. За smoothTime
    *	проходится примерно две трети оставшегося пути.
    */
    inline XYZEngine::Vector2Df ApproachPoint(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to,
        float smoothTime, float deltaTime)
    {
        if (smoothTime <= 0.f || deltaTime <= 0.f)
        {
            return to;
        }

        float part = 1.f - std::exp(-deltaTime / smoothTime);

        return from + (to - from) * part;
    }

    /**
    *	Держит центр кадра так, чтобы за краем карты не было черноты.
    *
    *	Карта уже кадра - центрируем её: зажимать нечего, и без этого камера
    *	билась бы между двумя границами, которые противоречат друг другу.
    */
    inline float ClampAxis(float center, float halfView, float min, float max)
    {
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
