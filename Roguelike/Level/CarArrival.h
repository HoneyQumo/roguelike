#pragma once

#include <algorithm>
#include <cmath>
#include <MathUtils.h>
#include <Vector.h>
#include "GameSettings.h"

namespace RoguelikeGame
{
    struct CarPose
    {
        XYZEngine::Vector2Df place;
        float angle = 0.f;
    };

    /**
    *	Машина влетает справа, тормозит и уходит в занос, разворачиваясь носом
    *	по ходу дальнейшего пути. Часть пути - доля от нуля до единицы.
    *
    *	Вынесено из сцены отдельной функцией: подобрать кривую торможения на глаз
    *	нельзя, а так её видно по числам.
    */
    inline CarPose ArrivalPose(const XYZEngine::Vector2Df& park, float part)
    {
        part = std::clamp(part, 0.f, 1.f);

        // Торможение: почти весь путь проезжает сразу, последние метры ползёт.
        float left = 1.f - part;
        float passed = 1.f - left * left * left;

        // Занос начинается на торможении, а не с первого метра - иначе это разворот.
        float swing = part <= ARRIVAL_SKID_START
            ? 0.f
            : (part - ARRIVAL_SKID_START) / (1.f - ARRIVAL_SKID_START);
        float turn = swing * swing * (3.f - 2.f * swing);

        CarPose pose;
        pose.place.x = park.x + ARRIVAL_ENTRY_OFFSET * (1.f - passed);
        pose.place.y = park.y + ARRIVAL_SKID_SLIDE * std::sin(turn * XYZEngine::PI);
        pose.angle = ARRIVAL_FACING_IN * (1.f - turn);

        return pose;
    }
}
