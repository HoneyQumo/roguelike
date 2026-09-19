#pragma once

#include <functional>
#include <string>
#include <vector>
#include <Component.h>
#include <EventList.h>
#include <Vector.h>
#include "LevelData.h"
#include "LevelZones.h"

namespace RoguelikeGame
{
    /**
    *	Засада: вошёл в комнату - из-за угла вышли. Выход она не запирает,
    *	это неприятность, а не ворота, и тем отличается от волны.
    *
    *	Срабатывает по входу игрока в зону, а не по пробуждению комнаты:
    *	RoomWakeComponent будит на комнату вперёд, и засада вышла бы заранее.
    */
    class AmbushComponent : public XYZEngine::Component
    {
    public:
        using Spawner = std::function<XYZEngine::GameObject*(TileType, const XYZEngine::Vector2Df&)>;

        AmbushComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetTargetName(const std::string& newTargetName);
        void SetZone(const LevelZone& newZone);
        void SetSpec(const AmbushSpec& newSpec);
        void SetPoints(std::vector<XYZEngine::Vector2Df> newPoints);
        void SetSpawner(Spawner newSpawner);

        const std::string& GetZoneId() const;
        bool IsArmed() const;
        bool IsSprung() const;
        int GetSpawnedCount() const;

        void Arm();
        void Spring();

        XYZEngine::SubscriptionId SubscribeSprung(std::function<void(int)> onSprung);

    private:
        std::string targetName;
        LevelZone zone;
        AmbushSpec spec;
        std::vector<XYZEngine::Vector2Df> points;
        Spawner spawner;

        bool isArmed = false;
        bool isSprung = false;
        float waited = 0.f;
        int spawnedCount = 0;
        std::size_t nextPoint = 0;

        XYZEngine::EventList<int> sprungEvent;

        bool IsTargetInside() const;
        XYZEngine::Vector2Df PickPoint();
    };
}
