#pragma once

#include <algorithm>
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
    // Ниже этого звук уже не различить среди прочего: иначе шаг за тремя
    // стенами формально слышно, хотя он не значит ничего.
    constexpr float NOISE_HEARD_AT = 0.05f;
    // На краю радиуса звук ещё слышен, но еле-еле: радиус обязан остаться
    // тем, чем был, - границей слышимости, а не границей внимания.
    constexpr float NOISE_EDGE_LOUDNESS = 0.1f;
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
        // Радиус - докуда дотягивается, сила - насколько громко у источника.
        // Слить в одно нельзя: у глушителя радиус мал, но это не шёпот.
        float radius = 0.f;
        float loudness = 1.f;
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

    // 0 - не слышно вовсе, 1 - вплотную к источнику полной силы.
    inline float LoudnessAt(const Noise& noise, const XYZEngine::Vector2Df& listener, Faction listenerSide,
        int wallsBetween = 0)
    {
        if (noise.radius <= 0.f || noise.loudness <= 0.f || !ReachesSide(noise, listenerSide))
        {
            return 0.f;
        }

        float reach = MuffledRadius(noise.radius, wallsBetween);
        if (reach <= 0.f)
        {
            return 0.f;
        }

        float distance = (listener - noise.position).GetLength();
        if (distance > reach)
        {
            return 0.f;
        }

        float muffle = reach / noise.radius;
        float fade = 1.f - (1.f - NOISE_EDGE_LOUDNESS) * (distance / reach);

        return std::min(noise.loudness * muffle * fade, 1.f);
    }

    inline bool IsHeard(const Noise& noise, const XYZEngine::Vector2Df& listener, Faction listenerSide, int wallsBetween = 0)
    {
        return LoudnessAt(noise, listener, listenerSide, wallsBetween) > NOISE_HEARD_AT;
    }

    void RaiseNoise(const Noise& noise, const XYZEngine::GameObject* except = nullptr);

    // Шум разносится только врагам: игроку нужен отдельный вход.
    // Список переживает смену локации и рестарт, так что отписываться обязательно.
    XYZEngine::SubscriptionId SubscribeNoiseRaised(std::function<void(const Noise&)> onNoise);
    void UnsubscribeNoiseRaised(XYZEngine::SubscriptionId subscription);
    void ClearNoiseListeners();

    std::size_t NoiseListenerCount();
}
