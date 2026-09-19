#pragma once

#include <string>

namespace RoguelikeGame
{
    enum class TrapKind
    {
        None,
        // Шипы: бьют того, кто наступил.
        Hurt,
        // Растяжка: не бьёт, а зовёт охрану. Для тихого пути это хуже удара.
        Alarm
    };

    inline TrapKind TrapKindFrom(const std::string& name)
    {
        if (name == "hurt")
        {
            return TrapKind::Hurt;
        }

        if (name == "alarm")
        {
            return TrapKind::Alarm;
        }

        return TrapKind::None;
    }
}
