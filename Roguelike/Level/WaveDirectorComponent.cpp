#include "WaveDirectorComponent.h"
#include "GameSettings.h"
#include "HealthComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    // Кто и как рождает врагов, решает тот, кто строит уровень: компоненту это знать незачем.
    WaveDirectorComponent::WaveDirectorComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void WaveDirectorComponent::Update(float deltaTime)
    {
        if (isCleared || waves.empty() || points.empty())
        {
            return;
        }

        if (!isWaiting)
        {
            if (CountAlive() > 0)
            {
                return;
            }

            waveClearedEvent.Invoke(currentWave + 1, static_cast<int>(waves.size()));

            if (currentWave + 1 >= static_cast<int>(waves.size()))
            {
                isCleared = true;
                LOG_INFO("Waves are over");
                clearedEvent.Invoke();
                return;
            }

            isWaiting = true;
            pause.Start(waves[currentWave + 1].delay);
            return;
        }

        pause.Tick(deltaTime);
        if (pause.IsRunning())
        {
            return;
        }

        StartWave();
    }

    void WaveDirectorComponent::Render()
    {
    }

    void WaveDirectorComponent::SetWaves(std::vector<WaveSpec> newWaves)
    {
        waves = std::move(newWaves);

        if (!waves.empty())
        {
            pause.Start(waves.front().delay);
        }
    }

    void WaveDirectorComponent::SetPoints(std::vector<XYZEngine::Vector2Df> newPoints)
    {
        points = std::move(newPoints);
    }

    void WaveDirectorComponent::SetSpawner(Spawner newSpawner)
    {
        if (newSpawner != nullptr)
        {
            spawner = std::move(newSpawner);
        }
    }

    void WaveDirectorComponent::SetHero(XYZEngine::GameObject* newHero)
    {
        hero = newHero;
    }

    int WaveDirectorComponent::GetCurrentWave() const
    {
        return currentWave;
    }

    int WaveDirectorComponent::GetWavesCount() const
    {
        return static_cast<int>(waves.size());
    }

    int WaveDirectorComponent::CountAlive() const
    {
        int alive = 0;
        for (XYZEngine::GameObject* enemy : spawned)
        {
            if (enemy == nullptr)
            {
                continue;
            }

            auto health = enemy->GetComponent<HealthComponent>();
            if (health != nullptr && health->IsAlive())
            {
                alive++;
            }
        }

        return alive;
    }

    bool WaveDirectorComponent::IsCleared() const
    {
        return isCleared;
    }

    XYZEngine::SubscriptionId WaveDirectorComponent::SubscribeWaveStarted(std::function<void(int, int)> onWaveStarted)
    {
        return waveStartedEvent.Subscribe(std::move(onWaveStarted));
    }

    XYZEngine::SubscriptionId WaveDirectorComponent::SubscribeWaveCleared(std::function<void(int, int)> onWaveCleared)
    {
        return waveClearedEvent.Subscribe(std::move(onWaveCleared));
    }

    XYZEngine::SubscriptionId WaveDirectorComponent::SubscribeEnemySpawned(std::function<void(XYZEngine::GameObject*)> onEnemySpawned)
    {
        return enemySpawnedEvent.Subscribe(std::move(onEnemySpawned));
    }

    XYZEngine::SubscriptionId WaveDirectorComponent::SubscribeCleared(std::function<void()> onCleared)
    {
        return clearedEvent.Subscribe(std::move(onCleared));
    }

    void WaveDirectorComponent::StartWave()
    {
        currentWave++;
        isWaiting = false;
        spawned.clear();

        const WaveSpec& wave = waves[currentWave];
        for (const WaveEntry& entry : wave.entries)
        {
            for (int index = 0; index < entry.count; index++)
            {
                SpawnOne(entry.enemy);
            }
        }

        LOG_INFO("Wave " + std::to_string(currentWave + 1) + " of " + std::to_string(waves.size())
            + " starts with " + std::to_string(spawned.size()) + " enemies");

        waveStartedEvent.Invoke(currentWave + 1, static_cast<int>(waves.size()));
    }

    void WaveDirectorComponent::SpawnOne(TileType enemy)
    {
        if (spawner == nullptr)
        {
            return;
        }

        XYZEngine::GameObject* born = spawner(enemy, PickPoint());
        if (born == nullptr)
        {
            return;
        }

        spawned.push_back(born);
        enemySpawnedEvent.Invoke(born);
    }

    // Точки перебираются по кругу, но прямо под ногами враг не появляется.
    XYZEngine::Vector2Df WaveDirectorComponent::PickPoint()
    {
        XYZEngine::Vector2Df chosen = points[nextPoint % points.size()];

        for (std::size_t step = 0u; step < points.size(); step++)
        {
            const XYZEngine::Vector2Df& candidate = points[(nextPoint + step) % points.size()];
            if (hero == nullptr || (candidate - hero->GetTransform()->GetWorldPosition()).GetLength() >= WAVE_SPAWN_GAP)
            {
                chosen = candidate;
                nextPoint = (nextPoint + step + 1u) % points.size();
                return chosen;
            }
        }

        nextPoint = (nextPoint + 1u) % points.size();

        return chosen;
    }
}
