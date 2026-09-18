#pragma once

#include <Vector.h>

namespace RoguelikeGame
{
    /**
    *	Два знака и два смысла: «?» - что-то происходит, «!» - тебя ведут.
    *	Шум попадает в «?» по той же причине: игрок слышал, но не видел.
    */
    enum class ThreatKind
    {
        Alerted,
        Provoked,
        Noise
    };

    struct ThreatSource
    {
        XYZEngine::Vector2Df direction;
        ThreatKind kind = ThreatKind::Alerted;

        // Сколько источнику светить: у противника единица, пока он держит игрока, у шума тает.
        float strength = 1.f;
    };

    inline bool IsAlarming(ThreatKind kind)
    {
        return kind == ThreatKind::Provoked;
    }
}
