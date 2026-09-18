#pragma once

#include <algorithm>
#include "GameSettings.h"

namespace RoguelikeGame
{
    /**
    *	Загорается быстрее, чем гаснет: вход в комнату должен быть мгновенным,
    *	а память - оседать мягко, иначе шаг через границу клетки читается щелчком.
    */
    inline float ApproachLight(float shown, float target, float deltaTime)
    {
        if (deltaTime <= 0.f || shown == target)
        {
            return shown;
        }

        float step = (target > shown ? FOG_FADE_IN_SPEED : FOG_FADE_OUT_SPEED) * deltaTime;

        return target > shown ? std::min(shown + step, target) : std::max(shown - step, target);
    }
}
