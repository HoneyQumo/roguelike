#pragma once

#include <functional>
#include <string>
#include <vector>
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
    class SpriteMovementAnimationComponent;
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
        int GetAbilityUses(BossAbility ability) const;
        XYZEngine::Vector2Df GetCastPoint() const;
        void DetonateBlast(const XYZEngine::Vector2Df& center);
        void RegisterMinion(XYZEngine::GameObject* minion);

        XYZEngine::SubscriptionId SubscribeStateChanged(std::function<void(BossState, BossState)> onStateChanged);
        XYZEngine::SubscriptionId SubscribeAbilityUsed(std::function<void(BossAbility)> onAbilityUsed);
        XYZEngine::SubscriptionId SubscribeEnraged(std::function<void()> onEnraged);
        XYZEngine::SubscriptionId SubscribeShot(std::function<void(const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&, float, float)> onShot);
        XYZEngine::SubscriptionId SubscribeSummon(std::function<void(const XYZEngine::Vector2Df&)> onSummon);
        XYZEngine::SubscriptionId SubscribeBlast(std::function<void(const XYZEngine::Vector2Df&, float)> onBlast);
        XYZEngine::SubscriptionId SubscribeCastMark(std::function<void(const XYZEngine::Vector2Df&, float, float)> onCastMark);
        XYZEngine::SubscriptionId SubscribeMinionSpawned(std::function<void(XYZEngine::GameObject*)> onMinionSpawned);

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        XYZEngine::MovementComponent* movement = nullptr;
        XYZEngine::SpriteMovementAnimationComponent* animation = nullptr;
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
        bool hasFired = false;
        float baseSpeed = 0.f;
        XYZEngine::Vector2Df dashDirection = {1.f, 0.f};
        XYZEngine::Vector2Df aimDirection = {1.f, 0.f};
        XYZEngine::Vector2Df targetPosition = {0.f, 0.f};
        XYZEngine::Vector2Df castPoint = {0.f, 0.f};
        std::vector<XYZEngine::GameObject*> minions;
        int abilityUses[BOSS_ABILITY_SLOTS] = {0, 0};

        XYZEngine::Cooldown actionTimer;
        XYZEngine::Cooldown recoveryTimer;
        XYZEngine::Cooldown roarTimer;
        XYZEngine::Cooldown abilityCooldown[BOSS_ABILITY_SLOTS];

        XYZEngine::EventList<BossState, BossState> stateChangedEvent;
        XYZEngine::EventList<BossAbility> abilityUsedEvent;
        XYZEngine::EventList<> enragedEvent;
        XYZEngine::EventList<const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&, float, float> shotEvent;
        XYZEngine::EventList<const XYZEngine::Vector2Df&> summonEvent;
        XYZEngine::EventList<const XYZEngine::Vector2Df&, float> blastEvent;
        XYZEngine::EventList<const XYZEngine::Vector2Df&, float, float> castMarkEvent;
        XYZEngine::EventList<XYZEngine::GameObject*> minionSpawnedEvent;

        XYZEngine::GameObject* FindTarget() const;
        float DistanceTo(const XYZEngine::GameObject& target) const;
        BossAbility ChooseAction(float distance) const;
        bool IsAbilityReady(int slot, BossAbility ability) const;
        float PaceScale() const;
        float DamageScale() const;
        int CountAliveMinions() const;
        void TickTimers(float deltaTime);
        void EnterState(BossState next);
        void ExitState(BossState previous);
        void UpdateAttack();
        void FireAbility();
        void CastVolley(const BossAbilitySpec& spec);
        void CastSummon(const BossAbilitySpec& spec);
        void CastBlast(const BossAbilitySpec& spec);
        void BeginDash(const BossAbilitySpec& spec);
        void FinishDash(const BossAbilitySpec& spec);
        void DealAreaDamage(const XYZEngine::Vector2Df& center, float radius, float damage);
        void StopMoving();
    };
}
