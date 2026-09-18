#include "ThreatWatchComponent.h"
#include "ChaseComponent.h"
#include "FactionComponent.h"
#include "GameSettings.h"
#include "FogOfWar.h"
#include "LevelGrid.h"
#include <GameObject.h>
#include <GameWorld.h>

namespace RoguelikeGame
{
    ThreatWatchComponent::ThreatWatchComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    ThreatWatchComponent::~ThreatWatchComponent()
    {
        UnsubscribeNoiseRaised(noiseSubscription);
        noiseSubscription = XYZEngine::NO_SUBSCRIPTION;
    }

    void ThreatWatchComponent::Start()
    {
        // Start зовут и после смены локации: без снятия старой подписки их накопится по одной на уровень.
        UnsubscribeNoiseRaised(noiseSubscription);

        noiseSubscription = SubscribeNoiseRaised([this](const Noise& noise)
        {
            if (gameObject == nullptr || !IsEnabled())
            {
                return;
            }

            XYZEngine::Vector2Df place = gameObject->GetTransform()->GetWorldPosition();
            int walls = LevelGrid::Current().CountWallsBetween(noise.position, place);

            if (IsHeard(noise, place, GetFactionOf(gameObject), walls))
            {
                Hear(noise.position);
            }
        });
    }

    void ThreatWatchComponent::Update(float deltaTime)
    {
        sources.clear();

        // Без тумана скрывать нечего: знак говорил бы о том, что и так на экране.
        // Проверка здесь, а не у противников: шум их проверку не проходит.
        if (gameObject == nullptr || !FogOfWar::Current().IsEnabled())
        {
            pings.clear();
            return;
        }

        FadePings(deltaTime);
        CollectEnemies(gameObject->GetTransform()->GetWorldPosition());

        for (const Ping& ping : pings)
        {
            sources.push_back({ping.direction, ThreatKind::Noise,
                THREAT_NOISE_TIME > 0.f ? ping.timeLeft / THREAT_NOISE_TIME : 0.f});
        }
    }

    void ThreatWatchComponent::Hear(const XYZEngine::Vector2Df& place)
    {
        if (gameObject == nullptr)
        {
            return;
        }

        XYZEngine::Vector2Df from = gameObject->GetTransform()->GetWorldPosition();
        pings.push_back({place - from, THREAT_NOISE_TIME});
    }

    const std::vector<ThreatSource>& ThreatWatchComponent::GetSources() const
    {
        return sources;
    }

    // Метку даёт только тот, кто уже держит игрока: подозрение осталось бы утечкой.
    void ThreatWatchComponent::CollectEnemies(const XYZEngine::Vector2Df& place)
    {
        for (ChaseComponent* enemy : XYZEngine::GameWorld::Instance()->FindComponents<ChaseComponent>())
        {
            if (enemy == nullptr || !enemy->IsEnabled() || enemy->GetGameObject() == gameObject)
            {
                continue;
            }

            AwarenessState state = enemy->GetAwarenessState();
            if (state == AwarenessState::Calm)
            {
                continue;
            }

            // Над видимым противником и так висит шкала осведомлённости,
            // и второй указатель был бы лишним.
            XYZEngine::Vector2Df at = enemy->GetGameObject()->GetTransform()->GetWorldPosition();
            if (FogOfWar::Current().GetStateAt(at) == FogState::Seen)
            {
                continue;
            }

            ThreatKind kind = state == AwarenessState::Provoked ? ThreatKind::Provoked : ThreatKind::Alerted;
            sources.push_back({at - place, kind, 1.f});
        }
    }

    void ThreatWatchComponent::FadePings(float deltaTime)
    {
        for (Ping& ping : pings)
        {
            ping.timeLeft -= deltaTime;
        }

        pings.erase(std::remove_if(pings.begin(), pings.end(),
            [](const Ping& ping) { return ping.timeLeft <= 0.f; }), pings.end());
    }
}
