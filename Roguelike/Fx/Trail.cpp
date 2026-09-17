#include "Trail.h"
#include <algorithm>

using namespace XYZEngine;

namespace RoguelikeGame
{
    namespace
    {
        // На крутом повороте усы стыка уходят в бесконечность: длину приходится ограничивать.
        constexpr float TRAIL_MITRE_LIMIT = 2.5f;

        Vector2Df Across(const Vector2Df& along)
        {
            return {-along.y, along.x};
        }

        Vector2Df StepBetween(const std::vector<TrailPoint>& points, std::size_t from, std::size_t to)
        {
            return (points[to].place - points[from].place).Normalized({0.f, 0.f});
        }
    }

    Vector2Df TrailSide(const std::vector<TrailPoint>& points, std::size_t index)
    {
        if (points.size() < 2u || index >= points.size())
        {
            return {0.f, 0.f};
        }

        float half = 0.5f * points[index].width;

        if (index == 0u)
        {
            return Across(StepBetween(points, 0u, 1u)) * half;
        }

        if (index + 1u == points.size())
        {
            return Across(StepBetween(points, index - 1u, index)) * half;
        }

        Vector2Df before = StepBetween(points, index - 1u, index);
        Vector2Df after = StepBetween(points, index, index + 1u);
        Vector2Df middle = (before + after).Normalized({0.f, 0.f});

        if (middle.IsZero())
        {
            return Across(before) * half;
        }

        Vector2Df side = Across(middle);

        // Чем острее поворот, тем длиннее должен быть ус, чтобы лента не сузилась.
        float cosine = side.x * Across(before).x + side.y * Across(before).y;
        float stretch = cosine > 0.f ? 1.f / cosine : TRAIL_MITRE_LIMIT;

        return side * (half * std::min(stretch, TRAIL_MITRE_LIMIT));
    }

    std::vector<TrailQuad> BuildTrail(const std::vector<TrailPoint>& points)
    {
        std::vector<TrailQuad> quads;
        if (points.size() < 2u)
        {
            return quads;
        }

        std::vector<Vector2Df> sides;
        sides.reserve(points.size());
        for (std::size_t index = 0u; index < points.size(); index++)
        {
            sides.push_back(TrailSide(points, index));
        }

        for (std::size_t index = 0u; index + 1u < points.size(); index++)
        {
            TrailQuad quad;
            quad.corners[0] = points[index].place - sides[index];
            quad.corners[1] = points[index + 1u].place - sides[index + 1u];
            quad.corners[2] = points[index + 1u].place + sides[index + 1u];
            quad.corners[3] = points[index].place + sides[index];
            quad.alpha = 0.5f * (points[index].alpha + points[index + 1u].alpha);

            quads.push_back(quad);
        }

        return quads;
    }
}
