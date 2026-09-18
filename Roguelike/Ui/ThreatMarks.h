#pragma once

#include <algorithm>
#include <cmath>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <Vector.h>
#include "GameSettings.h"

namespace RoguelikeGame
{
    enum class ThreatKind
    {
        Enemy,
        Noise
    };

    struct ThreatSource
    {
        XYZEngine::Vector2Df direction;
        ThreatKind kind = ThreatKind::Enemy;

        // Сколько метке жить: у врага единица, пока он держит игрока, у шума тает.
        float strength = 1.f;
    };

    struct ThreatMark
    {
        int sector = 0;
        ThreatKind kind = ThreatKind::Enemy;
        float strength = 1.f;
    };

    /**
    *	Сектор по направлению: ноль смотрит вверх, дальше по часовой стрелке.
    *	Мир с осью Y вверх, поэтому угол берётся от севера.
    */
    inline int SectorOf(const XYZEngine::Vector2Df& direction, int sectors)
    {
        if (sectors <= 0 || (direction.x == 0.f && direction.y == 0.f))
        {
            return 0;
        }

        float degrees = std::atan2(direction.x, direction.y) * 180.f / 3.14159265f;
        float step = 360.f / sectors;
        int sector = static_cast<int>(std::lround(degrees / step));

        return ((sector % sectors) + sectors) % sectors;
    }

    /**
    *	Одна метка на сектор, а не по метке на источник: пять стрелок под одним углом
    *	читаются как каша, а игроку нужно «опасность оттуда», а не счёт врагов.
    *
    *	Враг вытесняет шум в своём секторе: увиденный враг важнее звука. Между равными
    *	побеждает громкий - слабеющий шум не должен затирать свежий.
    */
    inline std::vector<ThreatMark> BuildThreatMarks(const std::vector<ThreatSource>& sources, int sectors)
    {
        std::vector<ThreatMark> marks;
        if (sectors <= 0)
        {
            return marks;
        }

        for (const ThreatSource& source : sources)
        {
            if (source.strength <= 0.f)
            {
                continue;
            }

            int sector = SectorOf(source.direction, sectors);

            auto taken = std::find_if(marks.begin(), marks.end(),
                [sector](const ThreatMark& mark) { return mark.sector == sector; });

            if (taken == marks.end())
            {
                marks.push_back({sector, source.kind, std::min(source.strength, 1.f)});
                continue;
            }

            bool winsByKind = taken->kind == ThreatKind::Noise && source.kind == ThreatKind::Enemy;
            bool winsByStrength = taken->kind == source.kind && source.strength > taken->strength;

            if (winsByKind || winsByStrength)
            {
                taken->kind = source.kind;
                taken->strength = std::min(source.strength, 1.f);
            }
        }

        std::sort(marks.begin(), marks.end(),
            [](const ThreatMark& first, const ThreatMark& second) { return first.sector < second.sector; });

        return marks;
    }

    // Куда метка садится на экране: по краю прямоугольника в сторону своего сектора.
    inline sf::Vector2f ThreatMarkOffset(int sector, int sectors, float halfWidth, float halfHeight)
    {
        if (sectors <= 0)
        {
            return {0.f, 0.f};
        }

        float degrees = sector * 360.f / sectors;
        float radians = degrees * 3.14159265f / 180.f;

        float along = std::sin(radians);
        float up = std::cos(radians);

        // Экранный Y растёт вниз, поэтому север - это минус.
        return {along * halfWidth, -up * halfHeight};
    }
}
