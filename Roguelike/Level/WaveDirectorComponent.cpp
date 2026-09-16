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

        pause.Tick(deltaTime);

        if (isWaiting)
        {
            if (pause.IsRunning())
            {
                return;
            }

            StartWave();
            return;
        }

        if (!IsWaveOver())
        {
            return;
        }

        // Оторвался - не значит отбил: о победе сообщаем, только когда из волны никого не осталось.
        if (!isWaveReported && CountAlive() == 0)
        {
            isWaveReported = true;
            waveClearedEvent.Invoke(currentWave + 1, static_cast<int>(waves.size()));
        }

        if (currentWave + 1 >= static_cast<int>(waves.size()))
        {
            // Последнюю волну мало пережить: пока хвост дышит в спину, уезжать рано.
            if (CountAliveNearby() > 0)
            {
                return;
            }

            isCleared = true;
            LOG_INFO("Waves are over");
            clearedEvent.Invoke();
            return;
        }

        isWaiting = true;
        pause.Start(waves[currentWave + 1].delay);
    }

    /**
    *	Волна закрыта, когда рядом с игроком никого не осталось или когда он ушёл
    *	достаточно далеко. Без второго условия один отставший враг где-то позади
    *	вешал бы погоню навсегда.
    */
    bool WaveDirectorComponent::IsWaveOver() const
    {
        return CountAliveNearby() == 0 || HasAdvanced();
    }

    bool WaveDirectorComponent::HasAdvanced() const
    {
        if (!hasWaveStart || hero == nullptr)
        {
            return false;
        }

        return (hero->GetTransform()->GetWorldPosition() - waveStart).GetLength() >= WAVE_ADVANCE_STEP;
    }

    float WaveDirectorComponent::DistanceToHero(const XYZEngine::Vector2Df& place) const
    {
        if (hero == nullptr)
        {
            return 0.f;
        }

        return (place - hero->GetTransform()->GetWorldPosition()).GetLength();
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

    int WaveDirectorComponent::CountAliveNearby() const
    {
        if (hero == nullptr)
        {
            return CountAlive();
        }

        int alive = 0;
        for (XYZEngine::GameObject* enemy : spawned)
        {
            if (enemy == nullptr)
            {
                continue;
            }

            auto health = enemy->GetComponent<HealthComponent>();
            if (health == nullptr || !health->IsAlive())
            {
                continue;
            }

            if (DistanceToHero(enemy->GetTransform()->GetWorldPosition()) <= WAVE_KEEP_RANGE)
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

    bool WaveDirectorComponent::IsPause() const
    {
        return isWaiting && !isCleared;
    }

    float WaveDirectorComponent::GetPauseLeft() const
    {
        return isWaiting ? pause.GetLeft() : 0.f;
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
        isWaveReported = false;
        spawned.clear();

        hasWaveStart = hero != nullptr;
        if (hasWaveStart)
        {
            waveStart = hero->GetTransform()->GetWorldPosition();
        }

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

    /**
    *	Точка берётся из окна вокруг игрока: дальше WAVE_SPAWN_GAP, чтобы враг не
    *	вырос под ногами, и ближе WAVE_SPAWN_REACH, чтобы волна вышла на этом
    *	участке моста, а не там, куда игрок дойдёт через минуту.
    *	Точки внутри окна перебираются по кругу - волна не лезет из одной дыры.
    */
    XYZEngine::Vector2Df WaveDirectorComponent::PickPoint()
    {
        if (hero == nullptr)
        {
            XYZEngine::Vector2Df anywhere = points[nextPoint % points.size()];
            nextPoint = (nextPoint + 1u) % points.size();

            return anywhere;
        }

        // Если в окно ничего не попало, берём ближайшую за его краем: она хотя бы рядом.
        XYZEngine::Vector2Df fallback = points[0];
        float bestDistance = -1.f;

        for (std::size_t step = 0u; step < points.size(); step++)
        {
            std::size_t index = (nextPoint + step) % points.size();
            const XYZEngine::Vector2Df& candidate = points[index];
            float distance = DistanceToHero(candidate);

            if (distance >= WAVE_SPAWN_GAP && distance <= WAVE_SPAWN_REACH)
            {
                nextPoint = (index + 1u) % points.size();

                return candidate;
            }

            if (distance >= WAVE_SPAWN_GAP && (bestDistance < 0.f || distance < bestDistance))
            {
                bestDistance = distance;
                fallback = candidate;
            }
        }

        return fallback;
    }
}
