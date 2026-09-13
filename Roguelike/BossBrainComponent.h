#pragma once

#include <functional>
#include <string>
#include <Component.h>
#include <Cooldown.h>
#include <EventList.h>
#include <Vector.h>
#include "BossCatalog.h"
#include "EnemyConfig.h"

namespace XYZEngine
{
    class GameObject;
    class MovementComponent;
    class TransformComponent;
}

namespace RoguelikeGame
{
    class ChaseComponent;
    class HealthComponent;

    class BossBrainComponent : public XYZEngine::Component
    {
    public:
        BossBrainComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetDefinition(const BossDefinition* newDefinition);
        void SetConfig(const EnemyConfig& newConfig);
        void SetTargetName(const std::string& newTargetName);
        void SetBasicAttack(XYZEngine::Component* newBasicAttack);

        BossState GetState() const;
        BossAbility GetCurrentAbility() const;
        bool IsEnraged() const;

        XYZEngine::SubscriptionId SubscribeStateChanged(std::function<void(BossState, BossState)> onStateChanged);
        XYZEngine::SubscriptionId SubscribeAbilityUsed(std::function<void(BossAbility)> onAbilityUsed);
        XYZEngine::SubscriptionId SubscribeEnraged(std::function<void()> onEnraged);

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        XYZEngine::MovementComponent* movement = nullptr;
        ChaseComponent* chase = nullptr;
        HealthComponent* health = nullptr;
        XYZEngine::Component* basicAttack = nullptr;

        const BossDefinition* definition = nullptr;
        EnemyConfig config{};
        std::string targetName;

        BossState state = BossState::Idle;
        BossAbility currentAbility = BossAbility::None;
        BossAbility pendingAbility = BossAbility::None;
        bool isEnraged = false;

        XYZEngine::Cooldown actionTimer;
        XYZEngine::Cooldown recoveryTimer;
        XYZEngine::Cooldown roarTimer;

        XYZEngine::EventList<BossState, BossState> stateChangedEvent;
        XYZEngine::EventList<BossAbility> abilityUsedEvent;
        XYZEngine::EventList<> enragedEvent;

        XYZEngine::GameObject* FindTarget() const;
        float DistanceTo(const XYZEngine::GameObject& target) const;
        BossAbility ChooseAction(float distance) const;
        void TickTimers(float deltaTime);
        void EnterState(BossState next);
        void ExitState(BossState previous);
        void StopMoving();
    };
}
