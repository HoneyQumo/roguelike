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
        float alertTime = 0.f;
        float visionHalfAngle = 180.f;
        float alertHalfAngle = 180.f;
        float searchTime = 0.f;
        float lookTime = 0.f;
        float lookHalfSweep = 0.f;
        int searchRadius = 0;
        int searchSpotsMin = 0;
        int searchSpotsMax = 0;
        float searchLookTime = 0.f;
        float alertRadiusScale = 1.f;
        float awarenessGain = 1.f;
        float awarenessDecay = 0.7f;
    };
}
