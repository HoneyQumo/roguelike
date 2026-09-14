#pragma once

#include <algorithm>

namespace RoguelikeGame
{
    struct SightMemory
    {
        float alertLeft = 0.f;
        bool hasPoint = false;
    };

    constexpr bool IsSearching(const SightMemory& memory)
    {
        return memory.alertLeft > 0.f;
    }

    constexpr SightMemory Fade(SightMemory memory, float deltaTime)
    {
        memory.alertLeft = memory.alertLeft > deltaTime ? memory.alertLeft - deltaTime : 0.f;

        if (memory.alertLeft <= 0.f)
        {
            memory.hasPoint = false;
        }

        return memory;
    }

    constexpr SightMemory Alarm(SightMemory memory, float duration)
    {
        memory.alertLeft = std::max(memory.alertLeft, duration);

        return memory;
    }

    constexpr SightMemory Remember(SightMemory memory, float duration)
    {
        memory = Alarm(memory, duration);
        memory.hasPoint = memory.alertLeft > 0.f;

        return memory;
    }

    constexpr SightMemory Forget(SightMemory memory)
    {
        memory.hasPoint = false;

        return memory;
    }

    constexpr SightMemory GiveUp(SightMemory memory, float lookAround)
    {
        memory.hasPoint = false;
        memory.alertLeft = std::min(memory.alertLeft, std::max(0.f, lookAround));

        return memory;
    }
}
