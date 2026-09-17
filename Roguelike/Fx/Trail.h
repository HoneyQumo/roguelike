#pragma once

#include <vector>
#include <Vector.h>

namespace RoguelikeGame
{
    struct TrailPoint
    {
        XYZEngine::Vector2Df place;
        float width = 0.f;
        float alpha = 1.f;
    };

    // Углы идут кольцом: правый край в начале, правый в конце, левый в конце, левый в начале.
    struct TrailQuad
    {
        XYZEngine::Vector2Df corners[4];
        float alpha = 1.f;
    };

    /**
    *	Смещение от осевой к краю ленты в точке пути.
    *
    *	На стыке берётся усреднённое направление, а не направление одного отрезка:
    *	тогда соседние квады получают **одни и те же** углы, и шва между ними нет.
    *	Это и отличает ленту от череды отдельных штампов.
    */
    XYZEngine::Vector2Df TrailSide(const std::vector<TrailPoint>& points, std::size_t index);

    /**
    *	Собирает ленту: на каждую пару соседних точек - один квад.
    *
    *	Меньше двух точек - ленты нет: рисовать отрезок нулевой длины нечем.
    */
    std::vector<TrailQuad> BuildTrail(const std::vector<TrailPoint>& points);
}
