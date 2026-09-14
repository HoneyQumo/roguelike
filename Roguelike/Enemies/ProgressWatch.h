#pragma once

#include <algorithm>

namespace RoguelikeGame
{
    struct ProgressWatch
    {
        float windowLeft = 0.f;
        float travelled = 0.f;
        float wanted = 0.f;
        float blockedLeft = 0.f;
    };

    constexpr bool IsBlocked(const ProgressWatch& watch)
    {
        return watch.blockedLeft > 0.f;
    }

    constexpr ProgressWatch Unblock(ProgressWatch watch)
    {
        watch.blockedLeft = 0.f;
        watch.windowLeft = 0.f;
        watch.travelled = 0.f;
        watch.wanted = 0.f;

        return watch;
    }

    constexpr ProgressWatch WatchProgress(ProgressWatch watch, float moved, float wanted, float deltaTime,
        float window, float blockedTime, float minShare)
    {
        watch.blockedLeft = watch.blockedLeft > deltaTime ? watch.blockedLeft - deltaTime : 0.f;

        watch.travelled += moved;
        watch.wanted += wanted;
        watch.windowLeft += deltaTime;

        if (watch.windowLeft < window)
        {
            return watch;
        }

        if (watch.wanted > 0.f && watch.travelled < watch.wanted * minShare)
        {
            watch.blockedLeft = blockedTime;
        }

        watch.windowLeft = 0.f;
        watch.travelled = 0.f;
        watch.wanted = 0.f;

        return watch;
    }
}
