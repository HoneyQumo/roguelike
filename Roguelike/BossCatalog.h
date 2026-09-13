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
