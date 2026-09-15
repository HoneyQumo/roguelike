#pragma once

#include <vector>
#include <Vector.h>
#include "Awareness.h"

namespace RoguelikeGame
{
    constexpr float GAUGE_FLASH_TIME = 0.45f;
    constexpr int GAUGE_STEPS = 24;

    struct GaugeLook
    {
        bool isShown = false;
        bool isAlarm = false;
        float part = 0.f;
        float fade = 1.f;
    };

    GaugeLook LookFor(float awareness, AwarenessState state, float sinceSpotted);
    std::vector<XYZEngine::Vector2Df> SectorPoints(const XYZEngine::Vector2Df& center, float radius, float part, int steps);
}
