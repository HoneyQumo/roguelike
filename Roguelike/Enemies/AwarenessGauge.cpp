#include "AwarenessGauge.h"
#include <MathUtils.h>
#include <algorithm>

namespace RoguelikeGame
{
    GaugeLook LookFor(float awareness, AwarenessState state, float sinceSpotted)
    {
        GaugeLook look;

        if (state == AwarenessState::Provoked)
        {
            look.isAlarm = true;
            look.part = 1.f;
            look.isShown = sinceSpotted < GAUGE_FLASH_TIME;
            look.fade = look.isShown ? 1.f - sinceSpotted / GAUGE_FLASH_TIME : 0.f;

            return look;
        }

        look.part = AwarenessPart(awareness);
        look.isShown = look.part > 0.f;

        return look;
    }

    std::vector<XYZEngine::Vector2Df> SectorPoints(const XYZEngine::Vector2Df& center, float radius, float part, int steps)
    {
        std::vector<XYZEngine::Vector2Df> points;

        float filled = std::min(std::max(part, 0.f), 1.f);
        if (radius <= 0.f || steps <= 0 || filled <= 0.f)
        {
            return points;
        }

        int arcSteps = std::max(1, static_cast<int>(std::lround(steps * filled)));
        points.reserve(static_cast<std::size_t>(arcSteps) + 2u);
        points.push_back(center);

        for (int step = 0; step <= arcSteps; step++)
        {
            float along = filled * static_cast<float>(step) / static_cast<float>(arcSteps);
            float degrees = 90.f - along * 360.f;
            XYZEngine::Vector2Df edge = XYZEngine::DirectionFromDegrees(degrees);

            points.push_back({center.x + edge.x * radius, center.y + edge.y * radius});
        }

        return points;
    }
}
