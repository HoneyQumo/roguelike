#pragma once

#include <cmath>
#include <MathUtils.h>
#include <Vector.h>

namespace RoguelikeGame
{
    /**
    *	Удар в спину бьёт больнее.
    *
    *	Правило чисто геометрическое: смотрим, куда повёрнута цель и с какой стороны
    *	пришёл удар. Ни осведомлённости, ни тревоги, ни шанса - зайти за спину можно
    *	и подкравшись, и обкрутив врага в открытом бою, и то и другое заслужено.
    *
    *	Геометрия работает на любой цели: у босса и у разрушаемого ящика нет мозга,
    *	но поворот есть у всех, и отдельной ветки «а если мозга нет» не появляется.
    */

    // Задняя полусфера шириной 120 градусов: попасть надо постараться, но не пиксель в пиксель.
    constexpr float BACKSTAB_HALF_ANGLE = 60.f;

    /**
    *	Пришёл ли удар в спину.
    *
    *	hitDirection смотрит от атакующего к цели. Значит атакующий стоит позади,
    *	когда цель смотрит туда же, куда летит удар: скалярное произведение близко
    *	к единице. Лицом к лицу оно близко к минус единице.
    */
    inline bool IsBackstab(const XYZEngine::Vector2Df& targetForward, const XYZEngine::Vector2Df& hitDirection,
        float halfAngleDegrees = BACKSTAB_HALF_ANGLE)
    {
        if (targetForward.IsZero() || hitDirection.IsZero() || halfAngleDegrees <= 0.f)
        {
            return false;
        }

        float behind = targetForward.Normalized().DotProduct(hitDirection.Normalized());

        return behind >= std::cos(XYZEngine::ToRadians(halfAngleDegrees));
    }

    // Множитель меньше единицы не крит, а ослабление: такого удара в спину не бывает.
    inline float BackstabDamage(float damage, bool isBackstab, float critScale)
    {
        return isBackstab && critScale > 1.f ? damage * critScale : damage;
    }
}
