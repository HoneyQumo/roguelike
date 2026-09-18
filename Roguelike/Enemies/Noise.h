#pragma once

#include <functional>
#include <EventList.h>
#include <Vector.h>
#include "Faction.h"

namespace XYZEngine
{
    class GameObject;
}

namespace RoguelikeGame
{
    constexpr float WALL_MUFFLE = 0.45f;
    constexpr float SHOUT_RADIUS = 420.f;
    constexpr float RADIO_SHOUT_RADIUS = 1100.f;

    enum class NoiseKind
    {
        Disturbance,
        Call
    };

    struct Noise
    {
        XYZEngine::Vector2Df position = {0.f, 0.f};
        float radius = 0.f;
        Faction from = Faction::Neutral;
        NoiseKind kind = NoiseKind::Disturbance;
    };

    inline bool ReachesSide(const Noise& noise, Faction listenerSide)
    {
        if (noise.kind == NoiseKind::Call)
        {
            return noise.from == listenerSide;
        }

        return noise.from == Faction::Neutral || noise.from != listenerSide;
    }

    inline float MuffledRadius(float radius, int wallsBetween)
    {
        float left = radius;
        for (int wall = 0; wall < wallsBetween; wall++)
        {
            left *= WALL_MUFFLE;
        }

        return left;
    }

    inline bool IsHeard(const Noise& noise, const XYZEngine::Vector2Df& listener, Faction listenerSide, int wallsBetween = 0)
    {
        if (noise.radius <= 0.f)
        {
            return false;
        }

        if (!ReachesSide(noise, listenerSide))
        {
            return false;
        }

        float reach = MuffledRadius(noise.radius, wallsBetween);

        return (listener - noise.position).GetLengthSquared() <= reach * reach;
    }

    void RaiseNoise(const Noise& noise, const XYZEngine::GameObject* except = nullptr);

    // Шум разносится только врагам: игроку нужен отдельный вход.
    // Список переживает смену локации и рестарт, так что отписываться обязательно.
    XYZEngine::SubscriptionId SubscribeNoiseRaised(std::function<void(const Noise&)> onNoise);
    void UnsubscribeNoiseRaised(XYZEngine::SubscriptionId subscription);
    void ClearNoiseListeners();

    std::size_t NoiseListenerCount();
}
