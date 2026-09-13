#include "BossBrainComponent.h"
#include "AreaDamage.h"
#include "ChaseComponent.h"
#include "HealthComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <MathUtils.h>
#include <MovementComponent.h>
#include <SpriteMovementAnimationComponent.h>
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
        animation = gameObject->GetComponent<XYZEngine::SpriteMovementAnimationComponent>();

        baseSpeed = movement != nullptr ? movement->GetSpeed() : config.speed;

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
        input.isEnrageDue = !isEnraged && definition != nullptr && health != nullptr
            && health->GetHealthPercent() <= definition->enragePart;

        XYZEngine::GameObject* target = FindTarget();
        if (target != nullptr)
        {
            XYZEngine::Vector2Df toTarget = target->GetTransform()->GetWorldPosition() - transform->GetWorldPosition();
            aimDirection = toTarget.Normalized(aimDirection);

            float distance = toTarget.GetLength();
            input.isTargetDetected = distance <= config.detectionRadius;

            if (state == BossState::Chase && input.isTargetDetected)
            {
                input.chosen = ChooseAction(distance);
            }
        }

        if (state == BossState::Attack)
        {
            UpdateAttack();
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
        if (definition == nullptr)
        {
            return distance <= config.attackRange ? BossAbility::Basic : BossAbility::None;
        }

        return ChooseBossAbility(*definition, distance, config.attackRange,
            IsAbilityReady(0, definition->first), IsAbilityReady(1, definition->second));
    }

    bool BossBrainComponent::IsAbilityReady(int slot, BossAbility ability) const
    {
        if (!abilityCooldown[slot].IsReady())
        {
            return false;
        }

        return ability != BossAbility::Summon || CountAliveMinions() < BOSS_MINION_LIMIT;
    }

    float BossBrainComponent::PaceScale() const
    {
        return isEnraged ? BOSS_ENRAGE_PACE_SCALE : 1.f;
    }

    float BossBrainComponent::DamageScale() const
    {
        return isEnraged ? BOSS_ENRAGE_DAMAGE_SCALE : 1.f;
    }

    int BossBrainComponent::CountAliveMinions() const
    {
        int alive = 0;
        for (XYZEngine::GameObject* minion : minions)
        {
            if (minion == nullptr)
            {
                continue;
            }

            auto minionHealth = minion->GetComponent<HealthComponent>();
            if (minionHealth == nullptr || minionHealth->IsAlive())
            {
                alive++;
            }
        }

        return alive;
    }

    void BossBrainComponent::TickTimers(float deltaTime)
    {
        actionTimer.Tick(deltaTime);
        recoveryTimer.Tick(deltaTime);
        roarTimer.Tick(deltaTime);

        for (XYZEngine::Cooldown& cooldown : abilityCooldown)
        {
            cooldown.Tick(deltaTime);
        }
    }

    void BossBrainComponent::EnterState(BossState next)
    {
        if (chase != nullptr)
        {
            chase->SetEnabled(next == BossState::Chase);
        }

        if (next != BossState::Chase && next != BossState::Attack)
        {
            StopMoving();
        }

        switch (next)
        {
        case BossState::Attack:
        {
            currentAbility = pendingAbility == BossAbility::None ? BossAbility::Basic : pendingAbility;
            hasFired = false;
            StopMoving();

            const BossAbilitySpec* spec = FindBossAbility(currentAbility);
            if (spec == nullptr)
            {
                actionTimer.Start(BOSS_BASIC_ATTACK_TIME * PaceScale());

                if (basicAttack != nullptr)
                {
                    basicAttack->SetEnabled(true);
                }
            }
            else
            {
                actionTimer.Start((spec->windup + spec->duration) * PaceScale());

                if (definition != nullptr)
                {
                    int slot = BossAbilitySlot(*definition, currentAbility);
                    if (slot >= 0)
                    {
                        abilityCooldown[slot].Start(spec->cooldown * PaceScale());
                        abilityUses[slot]++;
                    }
                }
            }

            abilityUsedEvent.Invoke(currentAbility);
            break;
        }

        case BossState::Cooldown:
            recoveryTimer.Start(BOSS_RECOVERY_TIME * PaceScale());
            break;

        case BossState::Enraged:
            isEnraged = true;
            roarTimer.Start(BOSS_ENRAGE_ROAR_TIME);

            if (movement != nullptr)
            {
                movement->SetSpeed(baseSpeed * BOSS_ENRAGE_SPEED_SCALE);
            }

            LOG_INFO(gameObject->GetName() + " is enraged");
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

        const BossAbilitySpec* spec = FindBossAbility(currentAbility);
        if (spec != nullptr && currentAbility == BossAbility::Dash)
        {
            FinishDash(*spec);
        }

        currentAbility = BossAbility::None;
        hasFired = false;

        if (basicAttack != nullptr)
        {
            basicAttack->SetEnabled(false);
        }
    }

    void BossBrainComponent::UpdateAttack()
    {
        const BossAbilitySpec* spec = FindBossAbility(currentAbility);
        if (spec == nullptr)
        {
            return;
        }

        if (!hasFired && actionTimer.GetLeft() <= spec->duration * PaceScale())
        {
            hasFired = true;
            FireAbility();
        }

        if (hasFired && currentAbility == BossAbility::Dash && movement != nullptr)
        {
            movement->SetDirection(dashDirection);
        }
    }

    void BossBrainComponent::FireAbility()
    {
        const BossAbilitySpec* spec = FindBossAbility(currentAbility);
        if (spec == nullptr)
        {
            return;
        }

        switch (currentAbility)
        {
        case BossAbility::Volley:
            CastVolley(*spec);
            break;

        case BossAbility::Summon:
            CastSummon(*spec);
            break;

        case BossAbility::Blast:
            CastBlast(*spec);
            break;

        case BossAbility::Dash:
            BeginDash(*spec);
            break;

        default:
            break;
        }
    }

    void BossBrainComponent::CastVolley(const BossAbilitySpec& spec)
    {
        if (spec.count <= 0)
        {
            return;
        }

        if (animation != nullptr)
        {
            animation->PlayShoot();
        }

        XYZEngine::Vector2Df origin = transform->GetWorldPosition() + aimDirection * BOSS_MUZZLE_DISTANCE;
        float damage = config.attackDamage * spec.damageScale * DamageScale();
        float step = spec.count > 1 ? spec.coneDegrees / (spec.count - 1) : 0.f;

        for (int index = 0; index < spec.count; index++)
        {
            float angle = spec.count > 1 ? -0.5f * spec.coneDegrees + index * step : 0.f;
            shotEvent.Invoke(origin, XYZEngine::RotateByDegrees(aimDirection, angle), damage, config.projectileSpeed);
        }
    }

    void BossBrainComponent::CastSummon(const BossAbilitySpec& spec)
    {
        if (spec.count <= 0)
        {
            return;
        }

        if (animation != nullptr)
        {
            animation->PlayReload();
        }

        XYZEngine::Vector2Df center = transform->GetWorldPosition();
        for (int index = 0; index < spec.count; index++)
        {
            float angle = 360.f * index / spec.count;
            XYZEngine::Vector2Df offset = XYZEngine::RotateByDegrees({1.f, 0.f}, angle) * spec.radius;
            summonEvent.Invoke(center + offset);
        }

        LOG_INFO(gameObject->GetName() + " calls minions");
    }

    void BossBrainComponent::CastBlast(const BossAbilitySpec& spec)
    {
        if (animation != nullptr)
        {
            animation->PlayMelee();
        }

        XYZEngine::Vector2Df center = transform->GetWorldPosition();
        DealAreaDamage(center, spec.radius, config.attackDamage * spec.damageScale * DamageScale());
        blastEvent.Invoke(center, spec.radius);
    }

    void BossBrainComponent::BeginDash(const BossAbilitySpec& spec)
    {
        dashDirection = aimDirection;

        if (movement != nullptr)
        {
            movement->SetSpeed(baseSpeed * spec.speedScale);
        }
    }

    void BossBrainComponent::FinishDash(const BossAbilitySpec& spec)
    {
        if (movement != nullptr)
        {
            movement->SetSpeed(isEnraged ? baseSpeed * BOSS_ENRAGE_SPEED_SCALE : baseSpeed);
        }

        if (!hasFired)
        {
            return;
        }

        XYZEngine::Vector2Df center = transform->GetWorldPosition();
        DealAreaDamage(center, spec.radius, config.attackDamage * spec.damageScale * DamageScale());
        blastEvent.Invoke(center, spec.radius);
    }

    void BossBrainComponent::DealAreaDamage(const XYZEngine::Vector2Df& center, float radius, float damage)
    {
        AreaQuery query = QueryDamageArea(center, radius, gameObject);

        for (const AreaTarget& target : query.targets)
        {
            if (target.health == nullptr || !CanDamage(Faction::Enemy, target.faction))
            {
                continue;
            }

            DamageSource source;
            source.kind = DamageKind::Explosion;
            source.attackerId = gameObject->GetId();
            source.attackerName = gameObject->GetName();
            source.attackerFaction = Faction::Enemy;
            source.position = target.position;
            source.direction = target.direction;

            target.health->TakeDamage(damage, source);
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

    int BossBrainComponent::GetAbilityUses(BossAbility ability) const
    {
        if (definition == nullptr)
        {
            return 0;
        }

        int slot = BossAbilitySlot(*definition, ability);

        return slot < 0 ? 0 : abilityUses[slot];
    }

    void BossBrainComponent::RegisterMinion(XYZEngine::GameObject* minion)
    {
        if (minion == nullptr)
        {
            return;
        }

        minions.push_back(minion);
        minionSpawnedEvent.Invoke(minion);
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

    XYZEngine::SubscriptionId BossBrainComponent::SubscribeShot(
        std::function<void(const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&, float, float)> onShot)
    {
        return shotEvent.Subscribe(std::move(onShot));
    }

    XYZEngine::SubscriptionId BossBrainComponent::SubscribeSummon(std::function<void(const XYZEngine::Vector2Df&)> onSummon)
    {
        return summonEvent.Subscribe(std::move(onSummon));
    }

    XYZEngine::SubscriptionId BossBrainComponent::SubscribeBlast(std::function<void(const XYZEngine::Vector2Df&, float)> onBlast)
    {
        return blastEvent.Subscribe(std::move(onBlast));
    }

    XYZEngine::SubscriptionId BossBrainComponent::SubscribeMinionSpawned(std::function<void(XYZEngine::GameObject*)> onMinionSpawned)
    {
        return minionSpawnedEvent.Subscribe(std::move(onMinionSpawned));
    }
}
