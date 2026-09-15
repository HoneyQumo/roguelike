#include "RoomWakeComponent.h"
#include "LevelGrid.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <TransformComponent.h>
#include <algorithm>

namespace RoguelikeGame
{
    RoomWakeComponent::RoomWakeComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void RoomWakeComponent::Start()
    {
    }

    void RoomWakeComponent::Update(float deltaTime)
    {
        if (sleepers.empty() || targetName.empty())
        {
            return;
        }

        XYZEngine::GameObject* target = XYZEngine::GameWorld::Instance()->FindGameObject(targetName);
        if (target == nullptr)
        {
            return;
        }

        int column = 0;
        int row = 0;
        LevelGrid::Current().ToCell(target->GetTransform()->GetWorldPosition(), column, row);

        const LevelZone* zone = FindZoneAt(zones, column, row);
        if (zone != nullptr)
        {
            Wake(zone->id);
        }
    }

    void RoomWakeComponent::Render()
    {
    }

    void RoomWakeComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
    }

    void RoomWakeComponent::SetZones(std::vector<LevelZone> newZones)
    {
        zones = std::move(newZones);
    }

    void RoomWakeComponent::AddSleeper(const std::string& zoneId, XYZEngine::GameObject* sleeper)
    {
        if (zoneId.empty() || sleeper == nullptr)
        {
            return;
        }

        sleeper->SetActive(false);
        sleepers.push_back({zoneId, sleeper});
    }

    bool RoomWakeComponent::IsAsleep(const std::string& zoneId) const
    {
        return std::any_of(sleepers.begin(), sleepers.end(),
            [&zoneId](const Sleeper& sleeper) { return sleeper.zoneId == zoneId; });
    }

    int RoomWakeComponent::GetSleepingCount() const
    {
        return static_cast<int>(sleepers.size());
    }

    void RoomWakeComponent::Wake(const std::string& zoneId)
    {
        if (!IsAsleep(zoneId))
        {
            return;
        }

        int woken = 0;
        for (const Sleeper& sleeper : sleepers)
        {
            if (sleeper.zoneId == zoneId && sleeper.gameObject != nullptr)
            {
                sleeper.gameObject->SetActive(true);
                woken++;
            }
        }

        sleepers.erase(std::remove_if(sleepers.begin(), sleepers.end(),
            [&zoneId](const Sleeper& sleeper) { return sleeper.zoneId == zoneId; }), sleepers.end());

        LOG_INFO("Room " + zoneId + " wakes up, " + std::to_string(woken) + " enemies");
        wokenEvent.Invoke(zoneId);
    }

    XYZEngine::SubscriptionId RoomWakeComponent::SubscribeWoken(std::function<void(const std::string&)> onWoken)
    {
        return wokenEvent.Subscribe(std::move(onWoken));
    }
}
