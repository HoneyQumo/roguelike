#include "AmbushComponent.h"
#include "LevelGrid.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    AmbushComponent::AmbushComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void AmbushComponent::Start()
    {
    }

    void AmbushComponent::Update(float deltaTime)
    {
        if (isSprung)
        {
            return;
        }

        if (!isArmed)
        {
            if (IsTargetInside())
            {
                Arm();
            }

            return;
        }

        waited += deltaTime;
        if (waited >= spec.delay)
        {
            Spring();
        }
    }

    void AmbushComponent::Render()
    {
    }

    void AmbushComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
    }

    void AmbushComponent::SetZone(const LevelZone& newZone)
    {
        zone = newZone;
    }

    void AmbushComponent::SetSpec(const AmbushSpec& newSpec)
    {
        spec = newSpec;
    }

    void AmbushComponent::SetPoints(std::vector<XYZEngine::Vector2Df> newPoints)
    {
        points = std::move(newPoints);
    }

    void AmbushComponent::SetSpawner(Spawner newSpawner)
    {
        spawner = std::move(newSpawner);
    }

    const std::string& AmbushComponent::GetZoneId() const
    {
        return spec.zoneId;
    }

    bool AmbushComponent::IsArmed() const
    {
        return isArmed;
    }

    bool AmbushComponent::IsSprung() const
    {
        return isSprung;
    }

    int AmbushComponent::GetSpawnedCount() const
    {
        return spawnedCount;
    }

    void AmbushComponent::Arm()
    {
        if (isArmed || isSprung)
        {
            return;
        }

        isArmed = true;
        waited = 0.f;
    }

    void AmbushComponent::Spring()
    {
        if (isSprung)
        {
            return;
        }

        isSprung = true;

        if (spawner == nullptr || points.empty())
        {
            LOG_ERROR("Ambush in zone " + spec.zoneId + " has nowhere to come from");
            return;
        }

        for (const WaveEntry& entry : spec.entries)
        {
            for (int born = 0; born < entry.count; born++)
            {
                if (spawner(entry.enemy, PickPoint()) != nullptr)
                {
                    spawnedCount++;
                }
            }
        }

        LOG_INFO("Ambush in zone " + spec.zoneId + " sprung: " + std::to_string(spawnedCount) + " came out");
        sprungEvent.Invoke(spawnedCount);
    }

    XYZEngine::SubscriptionId AmbushComponent::SubscribeSprung(std::function<void(int)> onSprung)
    {
        return sprungEvent.Subscribe(std::move(onSprung));
    }

    bool AmbushComponent::IsTargetInside() const
    {
        XYZEngine::GameObject* target = XYZEngine::GameWorld::Instance()->FindGameObject(targetName);
        if (target == nullptr)
        {
            return false;
        }

        int column = 0;
        int row = 0;
        LevelGrid::Current().ToCell(target->GetTransform()->GetWorldPosition(), column, row);

        return zone.Contains(column, row);
    }

    // Точки перебираются по кругу: засада из одного угла - это не засада.
    XYZEngine::Vector2Df AmbushComponent::PickPoint()
    {
        XYZEngine::Vector2Df place = points[nextPoint % points.size()];
        nextPoint = (nextPoint + 1u) % points.size();

        return place;
    }
}
