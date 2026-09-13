#include "Enemy.h"
#include "BossBrainComponent.h"
#include "CharacterFactory.h"
#include "GameSettings.h"
#include "GameResources.h"
#include "WeaponSetup.h"
#include "EnemyAttackComponent.h"
#include "HealthBarComponent.h"
#include "BloodPool.h"
#include "Fx.h"
#include <GameWorld.h>
#include "ChaseComponent.h"
#include "WeaponComponent.h"
#include "MeleeWeaponComponent.h"
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateEnemy(const EnemyConfig& config, const XYZEngine::Vector2Df& position)
    {
        CharacterSpec spec;
        spec.objectName = config.objectName;
        spec.textureMapName = config.textureMapName;
        spec.position = position;
        spec.collisionLayer = ENEMY_COLLISION_LAYER;
        spec.speed = config.speed;
        spec.maxHealth = config.maxHealth;
        spec.armor = config.armor;
        spec.faction = Faction::Enemy;
        spec.weapon = config.weapon;
        spec.healthBarColor = {200, 60, 60};

        ChaseComponent* chase = nullptr;
        CharacterParts parts = CreateCharacter(spec, [&chase, &config](XYZEngine::GameObject* object)
        {
            chase = object->AddComponent<ChaseComponent>();
            chase->SetTargetName(PLAYER_OBJECT_NAME);
            chase->SetDetectionRadius(config.detectionRadius);
            chase->SetStopDistance(config.stopDistance);
        });

        auto gameObject = parts.gameObject;
        auto weaponLayer = parts.weapon;
        auto movement = parts.movement;
        auto collider = parts.collider;
        auto aim = parts.aim;
        auto animation = parts.animation;
        auto health = parts.health;
        auto hurtAudio = parts.hurtAudio;
        auto hitFlash = parts.hitFlash;

        aim->AimAtGameObject(PLAYER_OBJECT_NAME);
        aim->SetMaxDistance(config.detectionRadius);

        if (config.attackRange <= 0.f)
        {
            LOG_INFO(std::string(config.objectName) + " is unarmed and can't attack");
        }
        else if (IsMelee(config.weapon))
        {
            const MeleeDefinition* melee = FindMelee(config.weapon);

            auto meleeAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
            meleeAudio->SetVolume(MELEE_HIT_VOLUME);

            auto meleeWeapon = gameObject->AddComponent<MeleeWeaponComponent>();
            meleeWeapon->SetQuickAttack(MakeQuickAttack(melee->quick, config.attackDamage, config.attackCooldown));
            meleeWeapon->SetDefinition(melee);
            PlayEffectsOnMeleeHit(meleeWeapon, meleeAudio);

            auto attack = gameObject->AddComponent<EnemyAttackComponent>();
            attack->SetTargetName(PLAYER_OBJECT_NAME);
            attack->SetAttackRange(config.attackRange);
        }
        else
        {
            const WeaponDefinition& weaponDefinition = GetWeapon(config.weapon);

            auto shotAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
            shotAudio->SetSound(GameResources::GetWeaponSound(weaponDefinition.shotSound));
            shotAudio->SetVolume(SHOT_VOLUME);

            auto reloadAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
            reloadAudio->SetSound(GameResources::GetWeaponSound(weaponDefinition.reloadSound));
            reloadAudio->SetVolume(RELOAD_VOLUME);

            ShotProfile shot = MakeShotProfile(config.weapon, config.attackDamage, config.projectileSpeed, config.attackCooldown);

            auto weaponComponent = gameObject->AddComponent<WeaponComponent>();
            ApplyWeaponDefinition(weaponComponent, config.weapon, shot);
            PlayEffectsOnReload(weaponComponent, animation, reloadAudio);
            PlayEffectsOnShot(weaponComponent, shotAudio, animation, weaponLayer);
            SpawnProjectilesOnShot(weaponComponent);

            auto attack = gameObject->AddComponent<EnemyAttackComponent>();
            attack->SetTargetName(PLAYER_OBJECT_NAME);
            attack->SetAttackRange(config.attackRange);
        }

        auto meleeComponent = gameObject->GetComponent<MeleeWeaponComponent>();
        health->SubscribeDamage([animation, hurtAudio, hitFlash, meleeComponent](const DamageInfo& damage)
        {
            Fx::SpawnHitBurst(damage.source.position, damage.source.direction);

            if (meleeComponent != nullptr)
            {
                meleeComponent->CancelAttack();
            }

            animation->PlayHurt();
            hurtAudio->Play();
            hitFlash->Flash();
        });

        auto weaponComponent = gameObject->GetComponent<WeaponComponent>();
        health->SubscribeDeath([gameObject, animation, movement, chase, collider, aim, weaponComponent, meleeComponent](const DeathInfo& death)
        {
            if (weaponComponent != nullptr)
            {
                weaponComponent->CancelReload();
            }

            if (meleeComponent != nullptr)
            {
                meleeComponent->CancelAttack();
            }

            animation->PlayDeath();
            movement->SetSpeed(0.f);
            if (chase != nullptr)
            {
                chase->SetDetectionRadius(0.f);
            }

            collider->SetTrigger(true);
            aim->SetEnabled(false);

            gameObject->SetRenderLayer(CORPSE_RENDER_LAYER);
            BloodPool::Spawn(death.position, death.rotation);
        });

        gameObject->SetRenderLayer(ENEMY_RENDER_LAYER);

        LOG_INFO(std::string(config.objectName) + " created at " + std::to_string(static_cast<int>(position.x)) + ";" + std::to_string(static_cast<int>(position.y)));
        return gameObject;
    }

    XYZEngine::GameObject* CreateBoss(const EnemyConfig& config, const BossDefinition& definition, const XYZEngine::Vector2Df& position)
    {
        auto gameObject = CreateEnemy(config, position);

        auto healthBar = gameObject->GetComponent<HealthBarComponent>();
        if (healthBar != nullptr)
        {
            healthBar->SetSize(BOSS_HEALTH_BAR_WIDTH, BOSS_HEALTH_BAR_HEIGHT);
            healthBar->SetOffset(0.f, BOSS_HEALTH_BAR_OFFSET_Y);
            healthBar->SetColors(BOSS_HEALTH_BAR_COLOR, VITALS_HUD_BACK_COLOR);
            healthBar->SetAlwaysVisible(true);
        }

        auto brain = gameObject->AddComponent<BossBrainComponent>();
        brain->SetDefinition(&definition);
        brain->SetConfig(config);
        brain->SetTargetName(PLAYER_OBJECT_NAME);
        brain->SetBasicAttack(gameObject->GetComponent<EnemyAttackComponent>());

        LOG_INFO(std::string("Boss ") + definition.id + " created with health " + std::to_string(static_cast<int>(config.maxHealth)));
        return gameObject;
    }
}
