#pragma once

#include <vector>
#include <Component.h>
#include <EventList.h>
#include "Noise.h"
#include "ThreatMarks.h"

namespace RoguelikeGame
{
    /**
    *	Показывает не то, что спрятано, а то, что уже себя обнаружило: врага,
    *	который заметил игрока, и шум, который игрок услышал. Незамеченный враг
    *	метки не даёт - иначе туман войны отменяется прибором.
    */
    class ThreatWatchComponent : public XYZEngine::Component
    {
    public:
        ThreatWatchComponent(XYZEngine::GameObject* gameObject);

        ~ThreatWatchComponent() override;

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override {}

        const std::vector<ThreatSource>& GetSources() const;

        // Шум приходит событием, а не опросом: он мгновенный и гаснет сам.
        void Hear(const XYZEngine::Vector2Df& place, float loudness);

    private:
        struct Ping
        {
            XYZEngine::Vector2Df direction;
            float timeLeft = 0.f;
        };

        std::vector<ThreatSource> sources;
        std::vector<Ping> pings;
        XYZEngine::SubscriptionId noiseSubscription = XYZEngine::NO_SUBSCRIPTION;

        void CollectEnemies(const XYZEngine::Vector2Df& place);
        void FadePings(float deltaTime);
    };
}
