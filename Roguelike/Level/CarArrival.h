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

        // Сила заноса от нуля до единицы: ею же дозируется дым и след.
        float skid = 0.f;
    };

    /**
    *	Где стоят задние колёса при такой позе.
    *
    *	Смещение задано в системе кузова и поворачивается вместе с ним: в заносе машина
    *	стоит поперёк, и колёса оказываются совсем не там, где были бы у прямо идущей.
    */
    inline XYZEngine::Vector2Df WheelPlace(const CarPose& pose, bool isLeft)
    {
        XYZEngine::Vector2Df offset = {-CAR_REAR_AXLE_OFFSET, isLeft ? CAR_WHEEL_SPACING : -CAR_WHEEL_SPACING};

        return pose.place + XYZEngine::RotateByDegrees(offset, pose.angle);
    }

    /**
    *	Доворот на посадке: машина стоит поперёк дороги, а уехать ей направо.
    *
    *	Вращение идёт в ту же сторону, что и занос, поэтому конец записан как 360,
    *	а не как 0 - иначе машина отматывала бы обратно.
    */
    inline float BoardingAngle(float part)
    {
        part = std::clamp(part, 0.f, 1.f);
        float turn = part * part * (3.f - 2.f * part);

        return ARRIVAL_FACING_PARKED + (ESCAPE_FACING_OUT - ARRIVAL_FACING_PARKED) * turn;
    }

    /**
    *	Машина влетает справа, тормозит и уходит в занос, вставая поперёк
    *	дороги дверью к тому, кто бежит. Часть пути - доля от нуля до единицы.
    *
    *	Вынесено из сцены отдельной функцией: подобрать кривую торможения на глаз
    *	нельзя, а так её видно по числам.
    */
    inline CarPose ArrivalPose(const XYZEngine::Vector2Df& park, float part)
    {
        part = std::clamp(part, 0.f, 1.f);

        // Скорость падает ровно - это и есть торможение в пол: резче машина
        // встаёт ещё до заноса и весь занос уже ползёт.
        float left = 1.f - part;
        float passed = 1.f - left * left;

        // Занос начинается на торможении, а не с первого метра - иначе это разворот.
        float swing = part <= ARRIVAL_SKID_START
            ? 0.f
            : (part - ARRIVAL_SKID_START) / (1.f - ARRIVAL_SKID_START);
        float turn = swing * swing * (3.f - 2.f * swing);

        CarPose pose;
        pose.place.x = park.x + ARRIVAL_ENTRY_OFFSET * (1.f - passed);
        pose.place.y = park.y + ARRIVAL_SKID_SLIDE * std::sin(turn * XYZEngine::PI);
        pose.angle = ARRIVAL_FACING_IN + (ARRIVAL_FACING_PARKED - ARRIVAL_FACING_IN) * turn;
        pose.skid = turn;

        return pose;
    }
}
