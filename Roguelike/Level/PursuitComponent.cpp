#include "PursuitComponent.h"
#include "GameSettings.h"
#include "HealthComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <TransformComponent.h>
#include <randomizer.h>

namespace RoguelikeGame
{
    PursuitComponent::PursuitComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void PursuitComponent::Update(float deltaTime)
    {
        if (spec.IsEmpty() || points.empty() || hero == nullptr)
        {
            return;
        }

        respawn.Tick(deltaTime);
        ForgetTheDead();
        UpdatePressure(deltaTime);

        if (CountChasing() >= pressure)
        {
            isDue = false;
            return;
        }

        // Нехватку сначала замечаем и только потом отсчитываем паузу: иначе убитого
        // сменяют в тот же миг, когда до этого все были живы и таймер успел истечь.
        if (!isDue)
        {
            isDue = true;
            respawn.Start(spec.respawn);
            return;
        }

        if (respawn.IsRunning())
        {
            return;
        }

        isDue = false;
        SendOne();
    }

    void PursuitComponent::Render()
    {
    }

    void PursuitComponent::SetSpec(const PursuitSpec& newSpec)
    {
        spec = newSpec;
        pressure = spec.keep;
    }

    void PursuitComponent::SetPoints(std::vector<XYZEngine::Vector2Df> newPoints)
    {
        points = std::move(newPoints);
    }

    void PursuitComponent::SetSpawner(Spawner newSpawner)
    {
        if (newSpawner != nullptr)
        {
            spawner = std::move(newSpawner);
        }
    }

    void PursuitComponent::SetHero(XYZEngine::GameObject* newHero)
    {
        hero = newHero;
    }

    void PursuitComponent::SetRoute(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to)
    {
        routeFrom = from;
        routeTo = to;

        XYZEngine::Vector2Df way = to - from;
        routeLength = way.GetLength();
        heading = way.Normalized();
        farthest = 0.f;
    }

    // Сколько пути пройдено вдоль маршрута: боковые метания по мосту прогрессом не считаются.
    float PursuitComponent::TravelledBy(const XYZEngine::Vector2Df& place) const
    {
        XYZEngine::Vector2Df fromStart = place - routeFrom;

        return fromStart.x * heading.x + fromStart.y * heading.y;
    }

    float PursuitComponent::GetProgress() const
    {
        if (routeLength <= 0.f || hero == nullptr)
        {
            return 0.f;
        }

        float part = TravelledBy(hero->GetTransform()->GetWorldPosition()) / routeLength;

        return part < 0.f ? 0.f : (part > 1.f ? 1.f : part);
    }

    int PursuitComponent::GetPressure() const
    {
        return pressure;
    }

    int PursuitComponent::CountChasing() const
    {
        int alive = 0;
        for (XYZEngine::GameObject* enemy : chasing)
        {
            auto health = enemy == nullptr ? nullptr : enemy->GetComponent<HealthComponent>();
            alive += health != nullptr && health->IsAlive() ? 1 : 0;
        }

        return alive;
    }

    bool PursuitComponent::IsCloseBehind() const
    {
        if (hero == nullptr)
        {
            return false;
        }

        XYZEngine::Vector2Df here = hero->GetTransform()->GetWorldPosition();
        for (XYZEngine::GameObject* enemy : chasing)
        {
            auto health = enemy == nullptr ? nullptr : enemy->GetComponent<HealthComponent>();
            if (health == nullptr || !health->IsAlive())
            {
                continue;
            }

            if ((enemy->GetTransform()->GetWorldPosition() - here).GetLength() <= PURSUIT_CLOSE_RANGE)
            {
                return true;
            }
        }

        return false;
    }

    /**
    *	Кого посылать, решает пройденный путь: берётся последний эшелон, который
    *	уже начался, а внутри него бросается жребий по весам.
    */
    TileType PursuitComponent::PickEnemy() const
    {
        const PursuitEchelon* chosen = nullptr;
        float progress = GetProgress();

        for (const PursuitEchelon& echelon : spec.echelons)
        {
            if (echelon.fromPart <= progress || chosen == nullptr)
            {
                chosen = &echelon;
            }
        }

        if (chosen == nullptr || chosen->entries.empty())
        {
            return TileType::Empty;
        }

        int total = chosen->TotalWeight();
        if (total <= 0)
        {
            return chosen->entries.front().enemy;
        }

        int ticket = random<int>(1, total);
        for (const PursuitEntry& entry : chosen->entries)
        {
            ticket -= entry.weight;
            if (ticket <= 0)
            {
                return entry.enemy;
            }
        }

        return chosen->entries.back().enemy;
    }

    XYZEngine::SubscriptionId PursuitComponent::SubscribeEnemySpawned(std::function<void(XYZEngine::GameObject*)> onEnemySpawned)
    {
        return enemySpawnedEvent.Subscribe(std::move(onEnemySpawned));
    }

    void PursuitComponent::ForgetTheDead()
    {
        for (auto place = chasing.begin(); place != chasing.end();)
        {
            auto health = *place == nullptr ? nullptr : (*place)->GetComponent<HealthComponent>();
            place = health == nullptr || !health->IsAlive() ? chasing.erase(place) : place + 1;
        }
    }

    /**
    *	Напор растёт, пока игрок топчется на месте, и падает, стоит ему продвинуться.
    *	Это и гонит вперёд: обороняться в одной точке становится всё тяжелее.
    */
    void PursuitComponent::UpdatePressure(float deltaTime)
    {
        float travelled = TravelledBy(hero->GetTransform()->GetWorldPosition());

        if (travelled > farthest + PURSUIT_RELAX_STEP)
        {
            farthest = travelled;
            sinceProgress = 0.f;
            pressure = spec.keep;
            return;
        }

        sinceProgress += deltaTime;
        if (sinceProgress < PURSUIT_GROW_TIME)
        {
            return;
        }

        sinceProgress = 0.f;
        pressure = pressure < spec.grow ? pressure + 1 : spec.grow;
    }

    void PursuitComponent::SendOne()
    {
        if (spawner == nullptr)
        {
            return;
        }

        XYZEngine::Vector2Df place;
        if (!TryPickPoint(place))
        {
            return;
        }

        TileType kind = PickEnemy();
        if (kind == TileType::Empty)
        {
            return;
        }

        XYZEngine::GameObject* born = spawner(kind, place);
        if (born == nullptr)
        {
            return;
        }

        chasing.push_back(born);
        enemySpawnedEvent.Invoke(born);
    }

    /**
    *	Преследователь выходит только из-за спины: точка должна быть позади игрока
    *	вдоль маршрута и в окне - ближе видно, как он появляется, дальше не догонит.
    */
    bool PursuitComponent::TryPickPoint(XYZEngine::Vector2Df& outPoint)
    {
        XYZEngine::Vector2Df here = hero->GetTransform()->GetWorldPosition();
        float heroTravelled = TravelledBy(here);

        for (std::size_t step = 0u; step < points.size(); step++)
        {
            std::size_t index = (nextPoint + step) % points.size();
            const XYZEngine::Vector2Df& candidate = points[index];

            if (TravelledBy(candidate) >= heroTravelled)
            {
                continue;
            }

            float distance = (candidate - here).GetLength();
            if (distance < PURSUIT_SPAWN_GAP || distance > PURSUIT_SPAWN_REACH)
            {
                continue;
            }

            nextPoint = (index + 1u) % points.size();
            outPoint = candidate;

            return true;
        }

        return false;
    }
}
