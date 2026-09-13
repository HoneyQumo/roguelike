#pragma once

#include <string_view>
#include "EnemyConfig.h"

namespace RoguelikeGame
{
    enum class BossAbility
    {
        None,
        Basic,
        Volley,
        Dash,
        Summon,
        Blast
    };

    enum class BossState
    {
        Idle,
        Chase,
        Attack,
        Cooldown,
        Enraged,
        Death
    };

    constexpr int BOSS_ABILITY_SLOTS = 2;

    struct BossDefinition
    {
        const char* id;
        BossAbility first;
        BossAbility second;
        float enragePart;
    };

    inline constexpr BossDefinition BOSSES[] = {
        {"puppeteer", BossAbility::Summon, BossAbility::Blast, 0.35f},
        {"gravedigger", BossAbility::Blast, BossAbility::Dash, 0.35f},
        {"colossus", BossAbility::Dash, BossAbility::Volley, 0.40f}
    };

    constexpr const BossDefinition* FindBoss(std::string_view id)
    {
        if (id.empty())
        {
            return nullptr;
        }

        for (const BossDefinition& boss : BOSSES)
        {
            if (id == boss.id)
            {
                return &boss;
            }
        }

        return nullptr;
    }

    constexpr int BossAbilitySlot(const BossDefinition& boss, BossAbility ability)
    {
        if (ability == BossAbility::None || ability == BossAbility::Basic)
        {
            return -1;
        }

        if (ability == boss.first)
        {
            return 0;
        }

        if (ability == boss.second)
        {
            return 1;
        }

        return -1;
    }

    constexpr EnemyConfig ApplyBossScales(const EnemyConfig& base, float healthScale, float damageScale)
    {
        EnemyConfig scaled = base;
        scaled.maxHealth = base.maxHealth * healthScale;
        scaled.attackDamage = base.attackDamage * damageScale;

        return scaled;
    }

    struct BossAbilitySpec
    {
        BossAbility ability;
        float minDistance;
        float maxDistance;
        float windup;
        float duration;
        float cooldown;
        float damageScale;
        float radius;
        float speedScale;
        int count;
        float coneDegrees;
    };

    inline constexpr BossAbilitySpec BOSS_ABILITIES[] = {
        {BossAbility::Volley, 220.f, 700.f, 0.55f, 0.35f, 6.f, 0.8f, 0.f, 1.f, 7, 90.f},
        {BossAbility::Dash, 260.f, 620.f, 0.45f, 0.45f, 7.f, 2.f, 110.f, 4.f, 0, 0.f},
        {BossAbility::Summon, 0.f, 900.f, 0.80f, 0.40f, 14.f, 0.f, 90.f, 1.f, 3, 0.f},
        {BossAbility::Blast, 0.f, 420.f, 0.60f, 0.30f, 9.f, 1.5f, 190.f, 1.f, 0, 0.f}
    };

    constexpr int BOSS_MINION_LIMIT = 4;

    constexpr bool IsAimedAtPoint(BossAbility ability)
    {
        return ability == BossAbility::Blast;
    }

    constexpr const BossAbilitySpec* FindBossAbility(BossAbility ability)
    {
        for (const BossAbilitySpec& spec : BOSS_ABILITIES)
        {
            if (spec.ability == ability)
            {
                return &spec;
            }
        }

        return nullptr;
    }

    constexpr bool IsInBossAbilityRange(BossAbility ability, float distance)
    {
        const BossAbilitySpec* spec = FindBossAbility(ability);

        return spec != nullptr && distance >= spec->minDistance && distance <= spec->maxDistance;
    }

    constexpr BossAbility ChooseBossAbility(const BossDefinition& boss, float distance, float attackRange,
        bool isFirstReady, bool isSecondReady)
    {
        if (isFirstReady && IsInBossAbilityRange(boss.first, distance))
        {
            return boss.first;
        }

        if (isSecondReady && IsInBossAbilityRange(boss.second, distance))
        {
            return boss.second;
        }

        return distance <= attackRange ? BossAbility::Basic : BossAbility::None;
    }

    struct BossBrainInput
    {
        bool isAlive = true;
        bool isEnrageDue = false;
        bool isTargetDetected = false;
        bool isActionDone = false;
        bool isRecoveryDone = false;
        bool isRoarDone = false;
        BossAbility chosen = BossAbility::None;
    };

    constexpr BossState NextBossState(BossState current, const BossBrainInput& input)
    {
        if (!input.isAlive || current == BossState::Death)
        {
            return BossState::Death;
        }

        if (input.isEnrageDue && current != BossState::Enraged)
        {
            return BossState::Enraged;
        }

        switch (current)
        {
        case BossState::Idle:
            return input.isTargetDetected ? BossState::Chase : BossState::Idle;

        case BossState::Chase:
            if (!input.isTargetDetected)
            {
                return BossState::Idle;
            }

            return input.chosen == BossAbility::None ? BossState::Chase : BossState::Attack;

        case BossState::Attack:
            return input.isActionDone ? BossState::Cooldown : BossState::Attack;

        case BossState::Cooldown:
            return input.isRecoveryDone ? BossState::Chase : BossState::Cooldown;

        case BossState::Enraged:
            return input.isRoarDone ? BossState::Chase : BossState::Enraged;

        default:
            return current;
        }
    }
}
