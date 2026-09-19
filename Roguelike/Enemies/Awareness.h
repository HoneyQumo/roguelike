#pragma once

#include <algorithm>
#include "Vision.h"

namespace RoguelikeGame
{
    constexpr float AWARENESS_ALERT_AT = 1.f;
    constexpr float AWARENESS_PROVOKE_AT = 2.f;
    constexpr float AWARENESS_CLOSE_GAIN = 2.f;
    constexpr float AWARENESS_MOVING_GAIN = 1.5f;
    constexpr float AWARENESS_MOVING_SPEED = 40.f;
    constexpr float AWARENESS_SIDE_GAIN = 1.f;
    constexpr float AWARENESS_BACK_GAIN = 0.7f;
    // Услышанный шум поднимает до тревоги, как и раньше, а громкость
    // добавляет сверху: в упор - до провокации, издалека - едва заметно.
    constexpr float AWARENESS_NOISE_GAIN = 1.f;

    enum class AwarenessState
    {
        Calm,
        Alerted,
        Provoked
    };

    struct AwarenessRates
    {
        float gain = 1.f;
        float decay = 0.7f;
    };

    struct AwarenessSense
    {
        VisionBand band = VisionBand::None;
        bool isTargetMoving = false;
        bool keepsMemory = false;
        float distance = 0.f;
        float maxDistance = 0.f;
    };

    constexpr float Nearness(float distance, float maxDistance)
    {
        if (maxDistance <= 0.f || distance >= maxDistance)
        {
            return 0.f;
        }

        return distance <= 0.f ? 1.f : 1.f - distance / maxDistance;
    }

    constexpr float BandGain(VisionBand band)
    {
        if (band == VisionBand::Periphery)
        {
            return AWARENESS_SIDE_GAIN;
        }

        return band == VisionBand::Back ? AWARENESS_BACK_GAIN : 0.f;
    }

    constexpr float GainFor(const AwarenessRates& rates, const AwarenessSense& sense)
    {
        float closer = 1.f + (AWARENESS_CLOSE_GAIN - 1.f) * Nearness(sense.distance, sense.maxDistance);
        float moving = sense.isTargetMoving ? AWARENESS_MOVING_GAIN : 1.f;

        return rates.gain * BandGain(sense.band) * closer * moving;
    }

    constexpr float NextAwareness(float level, const AwarenessRates& rates, const AwarenessSense& sense, float deltaTime)
    {
        if (deltaTime <= 0.f)
        {
            return level;
        }

        float next = level;
        if (sense.band == VisionBand::Focus)
        {
            return AWARENESS_PROVOKE_AT;
        }

        if (sense.band != VisionBand::None)
        {
            next += GainFor(rates, sense) * deltaTime;
        }
        else if (!sense.keepsMemory)
        {
            next -= rates.decay * deltaTime;
        }

        return std::min(std::max(next, 0.f), AWARENESS_PROVOKE_AT);
    }

    constexpr AwarenessState StateOf(float level)
    {
        if (level >= AWARENESS_PROVOKE_AT)
        {
            return AwarenessState::Provoked;
        }

        return level >= AWARENESS_ALERT_AT ? AwarenessState::Alerted : AwarenessState::Calm;
    }

    constexpr bool IsSpottedNow(AwarenessState before, AwarenessState now)
    {
        return now == AwarenessState::Provoked && before != AwarenessState::Provoked;
    }

    constexpr float AwarenessPart(float level)
    {
        return std::min(std::max(level / AWARENESS_PROVOKE_AT, 0.f), 1.f);
    }
}
