#pragma once

#include <functional>
#include <vector>
#include <Component.h>
#include <Cooldown.h>
#include <EventList.h>
#include <Vector.h>
#include "LevelData.h"

namespace RoguelikeGame
{
    class WaveDirectorComponent : public XYZEngine::Component
    {
    public:
        using Spawner = std::function<XYZEngine::GameObject*(TileType, const XYZEngine::Vector2Df&)>;

        WaveDirectorComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetWaves(std::vector<WaveSpec> newWaves);
        void SetPoints(std::vector<XYZEngine::Vector2Df> newPoints);
        void SetSpawner(Spawner newSpawner);
        void SetHero(XYZEngine::GameObject* newHero);

        int GetCurrentWave() const;
        int GetWavesCount() const;
        int CountAlive() const;

        // Только те, кто ещё висит у игрока на хвосте: отставшие в счёт не идут.
        int CountAliveNearby() const;
        bool IsCleared() const;

        XYZEngine::SubscriptionId SubscribeWaveStarted(std::function<void(int, int)> onWaveStarted);
        XYZEngine::SubscriptionId SubscribeWaveCleared(std::function<void(int, int)> onWaveCleared);
        XYZEngine::SubscriptionId SubscribeEnemySpawned(std::function<void(XYZEngine::GameObject*)> onEnemySpawned);
        XYZEngine::SubscriptionId SubscribeCleared(std::function<void()> onCleared);

    private:
        std::vector<WaveSpec> waves;
        std::vector<XYZEngine::Vector2Df> points;
        std::vector<XYZEngine::GameObject*> spawned;

        Spawner spawner;
        XYZEngine::GameObject* hero = nullptr;

        XYZEngine::Cooldown pause;
        int currentWave = -1;
        std::size_t nextPoint = 0u;
        bool isCleared = false;
        bool isWaiting = true;
        bool isWaveReported = false;
        bool hasWaveStart = false;
        XYZEngine::Vector2Df waveStart = {0.f, 0.f};

        XYZEngine::EventList<int, int> waveStartedEvent;
        XYZEngine::EventList<int, int> waveClearedEvent;
        XYZEngine::EventList<XYZEngine::GameObject*> enemySpawnedEvent;
        XYZEngine::EventList<> clearedEvent;

        void StartWave();
        void SpawnOne(TileType enemy);
        XYZEngine::Vector2Df PickPoint();
        bool IsWaveOver() const;
        bool HasAdvanced() const;
        float DistanceToHero(const XYZEngine::Vector2Df& place) const;
    };
}
