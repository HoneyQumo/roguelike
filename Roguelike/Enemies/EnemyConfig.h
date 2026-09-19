#pragma once

#include "Noise.h"
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
        // Ключ набора реплик в каталоге: сколько их там, знает сам набор.
        const char* spottedSpeech = nullptr;
        float peripheryHalfAngle = 0.f;
        float shoutRadius = SHOUT_RADIUS;
        float provokedRangeScale = 1.f;

        // Скорость налегке в погоне: настоящую даёт вес ствола. Ноль - бежит ровно так же, как патрулирует.
        float chaseSpeed = 0.f;

        // Градусов в секунду на разворот: без него спина живёт один кадр.
        float turnSpeed = 0.f;
    };
}
