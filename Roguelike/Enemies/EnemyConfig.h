#pragma once

#include "WeaponCatalog.h"

namespace RoguelikeGame
{
    struct EnemyConfig
    {
        const char* objectName;
        const char* textureMapName;

        WeaponId weapon = WeaponId::Knife;

        float speed = 0.f;
        float detectionRadius = 0.f;
        float stopDistance = 0.f;
        float maxHealth = 0.f;
        float armor = 0.f;

        float attackRange = 0.f;
        float attackDamage = 0.f;
        float attackCooldown = 0.f;
        float projectileSpeed = 0.f;

        const char* lootTable = nullptr;
    };
}
