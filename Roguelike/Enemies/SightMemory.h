#pragma once

#include <algorithm>

namespace RoguelikeGame
{
    struct SightMemory
    {
        float alertLeft = 0.f;
        float travelLeft = 0.f;
        bool hasPoint = false;
    };

    constexpr bool IsSearching(const SightMemory& memory)
    {
        return memory.alertLeft > 0.f;
    }

    /**
    *	Отсчитывает кадр памяти.
    *
    *	Пока враг в дороге к точке, тает бюджет на дорогу, а не тревога. Одним
    *	таймером на «сколько быть настороже» и «сколько идти» длинный обход не
    *	покрывался: тревога кончалась раньше пути, и враг разворачивался, не дойдя.
    *
    *	Дошёл или сдался - тает уже тревога, и это «осмотрелся и вернулся».
    */
    constexpr SightMemory Tick(SightMemory memory, float deltaTime, bool isOnTheWay)
    {
        if (memory.hasPoint && isOnTheWay)
        {
            memory.travelLeft = memory.travelLeft > deltaTime ? memory.travelLeft - deltaTime : 0.f;
            memory.hasPoint = memory.travelLeft > 0.f;

            return memory;
        }

        memory.alertLeft = memory.alertLeft > deltaTime ? memory.alertLeft - deltaTime : 0.f;

        if (memory.alertLeft <= 0.f)
        {
            memory.hasPoint = false;
            memory.travelLeft = 0.f;
        }

        return memory;
    }

    constexpr SightMemory Alarm(SightMemory memory, float duration)
    {
        memory.alertLeft = std::max(memory.alertLeft, duration);

        return memory;
    }

    constexpr SightMemory Remember(SightMemory memory, float duration, float travel)
    {
        memory = Alarm(memory, duration);
        memory.hasPoint = memory.alertLeft > 0.f;
        memory.travelLeft = memory.hasPoint ? std::max(memory.travelLeft, travel) : 0.f;

        return memory;
    }

    constexpr SightMemory Forget(SightMemory memory)
    {
        memory.hasPoint = false;
        memory.travelLeft = 0.f;

        return memory;
    }

    constexpr SightMemory GiveUp(SightMemory memory, float lookAround)
    {
        memory.hasPoint = false;
        memory.travelLeft = 0.f;
        memory.alertLeft = std::min(memory.alertLeft, std::max(0.f, lookAround));

        return memory;
    }
}
