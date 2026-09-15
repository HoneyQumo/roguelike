#pragma once

#include <string>
#include <vector>
#include <Component.h>
#include <EventList.h>
#include "LevelZones.h"

namespace RoguelikeGame
{
    class RoomWakeComponent : public XYZEngine::Component
    {
    public:
        RoomWakeComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetTargetName(const std::string& newTargetName);
        void SetZones(std::vector<LevelZone> newZones);
        void SetAhead(int newAhead);

        void AddSleeper(const std::string& zoneId, XYZEngine::GameObject* sleeper);

        bool IsAsleep(const std::string& zoneId) const;
        int GetSleepingCount() const;

        void Wake(const std::string& zoneId);

        XYZEngine::SubscriptionId SubscribeWoken(std::function<void(const std::string&)> onWoken);

    private:
        struct Sleeper
        {
            std::string zoneId;
            XYZEngine::GameObject* gameObject = nullptr;
        };

        std::string targetName;
        std::vector<LevelZone> zones;
        int ahead = 0;
        std::vector<Sleeper> sleepers;

        XYZEngine::EventList<const std::string&> wokenEvent;
    };
}
