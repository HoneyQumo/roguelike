#include "Enemy.h"
#include "CharacterFactory.h"
#include "GameSettings.h"
#include "GameResources.h"
#include "WeaponSetup.h"
#include "EnemyAttackComponent.h"
#include "BloodPool.h"
#include <GameWorld.h>
#include <ChaseComponent.h>
#include <WeaponComponent.h>
#include <MeleeWeaponComponent.h>
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
        spec.weapon = config.weapon;
        spec.healthBarColor = {200, 60, 60};

        XYZEngine::ChaseComponent* chase = nullptr;
        CharacterParts parts = CreateCharacter(spec, [&chase, &config](XYZEngine::GameObject* object)
        {
            chase = object->AddComponent<XYZEngine::ChaseComponent>();
            chase->SetTargetName("Player");
            chase->SetDetectionRadius(config.detectionRadius);
            chase->SetStopDistance(config.stopDistance);
        });

        auto gameObject = parts.gameObject;
        auto weaponLayer = parts.weapon;
        auto transform = parts.transform;
        auto movement = parts.movement;
        auto collider = parts.collider;
        auto aim = parts.aim;
        auto animation = parts.animation;
        auto health = parts.health;
        auto hurtAudio = parts.hurtAudio;
        auto hitFlash = parts.hitFlash;

        aim->AimAtGameObject("Player");
        aim->SetMaxDistance(config.detectionRadius);

        if (config.attackRange <= 0.f)
        {
            LOG_INFO(config.objectName + " is unarmed and can't attack");
        }
        else if (IsMelee(config.weapon))
        {
            const MeleeDefinition* melee = FindMelee(config.weapon);

            auto meleeAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
            meleeAudio->SetVolume(MELEE_HIT_VOLUME);

            auto meleeWeapon = gameObject->AddComponent<XYZEngine::MeleeWeaponComponent>();
            meleeWeapon->SetQuickAttack(MakeQuickAttack(melee->quick, config.attackDamage, config.attackCooldown));
            meleeWeapon->SetTargetName("Player");
            PlayEffectsOnMeleeHit(meleeWeapon, meleeAudio, melee);

            auto attack = gameObject->AddComponent<EnemyAttackComponent>();
            attack->SetTargetName("Player");
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

            auto weaponComponent = gameObject->AddComponent<XYZEngine::WeaponComponent>();
            ApplyWeaponDefinition(weaponComponent, config.weapon, shot);
            PlayEffectsOnReload(weaponComponent, animation, reloadAudio);
            PlayEffectsOnShot(weaponComponent, shotAudio, animation, weaponLayer);
            SpawnProjectilesOnShot(weaponComponent, config.objectName, config.weapon);

            auto attack = gameObject->AddComponent<EnemyAttackComponent>();
            attack->SetTargetName("Player");
            attack->SetAttackRange(config.attackRange);
        }

        auto meleeComponent = gameObject->GetComponent<XYZEngine::MeleeWeaponComponent>();
        health->SubscribeDamage([animation, hurtAudio, hitFlash, meleeComponent](float damage)
        {
            if (meleeComponent != nullptr)
            {
                meleeComponent->CancelAttack();
            }

            animation->PlayHurt();
            hurtAudio->Play();
            hitFlash->Flash();
        });

        auto weaponComponent = gameObject->GetComponent<XYZEngine::WeaponComponent>();
        health->SubscribeDeath([gameObject, transform, animation, movement, chase, collider, aim, weaponComponent, meleeComponent]()
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
            chase->SetDetectionRadius(0.f);
            collider->SetTrigger(true);
            aim->SetEnabled(false);

            gameObject->SetRenderLayer(CORPSE_RENDER_LAYER);
            BloodPool::Spawn(transform->GetWorldPosition(), transform->GetWorldRotation());
        });

        gameObject->SetRenderLayer(ENEMY_RENDER_LAYER);

        LOG_INFO(config.objectName + " created at " + std::to_string(static_cast<int>(position.x)) + ";" + std::to_string(static_cast<int>(position.y)));
        return gameObject;
    }
}
