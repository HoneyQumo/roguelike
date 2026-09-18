#pragma once

#include <Vector.h>

namespace RoguelikeGame
{
    /**
    *	Куда смотреть тому, кто идёт.
    *
    *	Цель и направление шага совпадают только на прямой дороге. На обходе вокруг
    *	стены враг шагает в одну сторону, а целился в другую - и смотрел сквозь стену
    *	на точку назначения.
    *
    *	Это не только некрасиво: конус зрения едет за прицелом, поэтому обойти такого
    *	врага со спины нельзя - спина у него там, где её не ждёшь.
    *
    *	Стоящий на месте смотрит на цель: шага нет, и направление брать неоткуда.
    */
    inline XYZEngine::Vector2Df LookAheadPoint(const XYZEngine::Vector2Df& position,
        const XYZEngine::Vector2Df& step, const XYZEngine::Vector2Df& goal, float distance)
    {
        if (step.IsZero() || distance <= 0.f)
        {
            return goal;
        }

        return position + step.Normalized() * distance;
    }
}
