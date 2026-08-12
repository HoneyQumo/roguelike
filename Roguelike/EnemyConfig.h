#pragma once

#include <string>

namespace RoguelikeGame
{
    struct EnemyConfig
    {
        std::string objectName;
        std::string textureMapName;

        int walkFirstFrame = 0;
        int walkFrames = 0;
        int idleFirstFrame = 0;
        int idleFrames = 0;
        int hurtFirstFrame = 0;
        int hurtFrames = 0;
        int deathFirstFrame = 0;
        int deathFrames = 0;
        std::string weaponTextureName;

        float speed = 0.f;
        float detectionRadius = 0.f;
        float stopDistance = 0.f;
        float maxHealth = 0.f;
        float armor = 0.f;

        float attackRange = 0.f;
        float attackDamage = 0.f;
        float attackCooldown = 0.f;
        float projectileSpeed = 0.f;
    };
}
