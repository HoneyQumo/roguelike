#include "BossBrainComponent.h"
#include "ChaseComponent.h"
#include "HealthComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <MovementComponent.h>
#include <TransformComponent.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    BossBrainComponent::BossBrainComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
    }

    void BossBrainComponent::Start()
    {
        movement = gameObject->GetComponent<XYZEngine::MovementComponent>();
        chase = gameObject->GetComponent<ChaseComponent>();
        health = gameObject->GetComponent<HealthComponent>();

        EnterState(state);
    }

    void BossBrainComponent::Update(float deltaTime)
    {
        TickTimers(deltaTime);

        BossBrainInput input;
        input.isAlive = health == nullptr || health->IsAlive();
        input.isActionDone = actionTimer.IsReady();
        input.isRecoveryDone = recoveryTimer.IsReady();
        input.isRoarDone = roarTimer.IsReady();

        XYZEngine::GameObject* target = FindTarget();
        if (target != nullptr)
        {
            float distance = DistanceTo(*target);
            input.isTargetDetected = distance <= config.detectionRadius;

            if (state == BossState::Chase && input.isTargetDetected)
            {
                input.chosen = ChooseAction(distance);
            }
        }

        BossState next = NextBossState(state, input);
        if (next == state)
        {
            return;
        }

        pendingAbility = input.chosen;

        BossState previous = state;
        ExitState(previous);
        state = next;
        EnterState(next);

        stateChangedEvent.Invoke(previous, next);
    }

    void BossBrainComponent::Render()
    {
    }

    XYZEngine::GameObject* BossBrainComponent::FindTarget() const
    {
        if (targetName.empty())
        {
            return nullptr;
        }

        XYZEngine::GameObject* target = XYZEngine::GameWorld::Instance()->FindGameObject(targetName);
        if (target == nullptr)
        {
            return nullptr;
        }

        auto targetHealth = target->GetComponent<HealthComponent>();

        return targetHealth == nullptr || targetHealth->IsAlive() ? target : nullptr;
    }

    float BossBrainComponent::DistanceTo(const XYZEngine::GameObject& target) const
    {
        return (target.GetTransform()->GetWorldPosition() - transform->GetWorldPosition()).GetLength();
    }

    BossAbility BossBrainComponent::ChooseAction(float distance) const
    {
        return distance <= config.attackRange ? BossAbility::Basic : BossAbility::None;
    }

    void BossBrainComponent::TickTimers(float deltaTime)
    {
        actionTimer.Tick(deltaTime);
        recoveryTimer.Tick(deltaTime);
        roarTimer.Tick(deltaTime);
    }

    void BossBrainComponent::EnterState(BossState next)
    {
        if (chase != nullptr)
        {
            chase->SetEnabled(next == BossState::Chase);
        }

        if (next != BossState::Chase)
        {
            StopMoving();
        }

        switch (next)
        {
        case BossState::Attack:
            currentAbility = pendingAbility == BossAbility::None ? BossAbility::Basic : pendingAbility;
            actionTimer.Start(BOSS_BASIC_ATTACK_TIME);

            if (basicAttack != nullptr && currentAbility == BossAbility::Basic)
            {
                basicAttack->SetEnabled(true);
            }

            abilityUsedEvent.Invoke(currentAbility);
            break;

        case BossState::Cooldown:
            recoveryTimer.Start(BOSS_RECOVERY_TIME);
            break;

        case BossState::Enraged:
            isEnraged = true;
            roarTimer.Start(BOSS_ENRAGE_ROAR_TIME);
            enragedEvent.Invoke();
            break;

        case BossState::Death:
            if (basicAttack != nullptr)
            {
                basicAttack->SetEnabled(false);
            }

            LOG_INFO(gameObject->GetName() + " is defeated");
            break;

        default:
            break;
        }
    }

    void BossBrainComponent::ExitState(BossState previous)
    {
        if (previous != BossState::Attack)
        {
            return;
        }

        currentAbility = BossAbility::None;

        if (basicAttack != nullptr)
        {
            basicAttack->SetEnabled(false);
        }
    }

    void BossBrainComponent::StopMoving()
    {
        if (movement != nullptr)
        {
            movement->SetDirection({0.f, 0.f});
        }
    }

    void BossBrainComponent::SetDefinition(const BossDefinition* newDefinition)
    {
        definition = newDefinition;
    }

    void BossBrainComponent::SetConfig(const EnemyConfig& newConfig)
    {
        config = newConfig;
    }

    void BossBrainComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
    }

    void BossBrainComponent::SetBasicAttack(XYZEngine::Component* newBasicAttack)
    {
        basicAttack = newBasicAttack;

        if (basicAttack != nullptr)
        {
            basicAttack->SetEnabled(false);
        }
    }

    BossState BossBrainComponent::GetState() const
    {
        return state;
    }

    BossAbility BossBrainComponent::GetCurrentAbility() const
    {
        return currentAbility;
    }

    bool BossBrainComponent::IsEnraged() const
    {
        return isEnraged;
    }

    XYZEngine::SubscriptionId BossBrainComponent::SubscribeStateChanged(std::function<void(BossState, BossState)> onStateChanged)
    {
        return stateChangedEvent.Subscribe(std::move(onStateChanged));
    }

    XYZEngine::SubscriptionId BossBrainComponent::SubscribeAbilityUsed(std::function<void(BossAbility)> onAbilityUsed)
    {
        return abilityUsedEvent.Subscribe(std::move(onAbilityUsed));
    }

    XYZEngine::SubscriptionId BossBrainComponent::SubscribeEnraged(std::function<void()> onEnraged)
    {
        return enragedEvent.Subscribe(std::move(onEnraged));
    }
}
