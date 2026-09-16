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
    /**
    *	Погоня. В отличие от волн, она ничего не ждёт и не кончается: за игроком
    *	держится несколько преследователей, убитого заменяет следующий.
    *	Стоять невыгодно - чем дольше игрок на месте, тем больше их на хвосте.
    */
    class PursuitComponent : public XYZEngine::Component
    {
    public:
        using Spawner = std::function<XYZEngine::GameObject*(TileType, const XYZEngine::Vector2Df&)>;

        PursuitComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetSpec(const PursuitSpec& newSpec);
        void SetPoints(std::vector<XYZEngine::Vector2Df> newPoints);
        void SetSpawner(Spawner newSpawner);
        void SetHero(XYZEngine::GameObject* newHero);

        // Куда игрок бежит: от этого зависит, что считать «позади» и сколько пути пройдено.
        void SetRoute(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to);

        float GetProgress() const;
        int GetPressure() const;
        int CountChasing() const;
        bool IsCloseBehind() const;
        TileType PickEnemy() const;

        XYZEngine::SubscriptionId SubscribeEnemySpawned(std::function<void(XYZEngine::GameObject*)> onEnemySpawned);

    private:
        PursuitSpec spec;
        std::vector<XYZEngine::Vector2Df> points;
        std::vector<XYZEngine::GameObject*> chasing;

        Spawner spawner;
        XYZEngine::GameObject* hero = nullptr;

        XYZEngine::Vector2Df routeFrom = {0.f, 0.f};
        XYZEngine::Vector2Df routeTo = {0.f, 0.f};
        XYZEngine::Vector2Df heading = {1.f, 0.f};
        float routeLength = 0.f;

        int pressure = 0;
        float farthest = 0.f;
        float sinceProgress = 0.f;
        std::size_t nextPoint = 0u;
        bool isDue = false;
        XYZEngine::Cooldown respawn;

        XYZEngine::EventList<XYZEngine::GameObject*> enemySpawnedEvent;

        void ForgetTheDead();
        void UpdatePressure(float deltaTime);
        void SendOne();
        bool TryPickPoint(XYZEngine::Vector2Df& outPoint);
        float TravelledBy(const XYZEngine::Vector2Df& place) const;
    };
}
