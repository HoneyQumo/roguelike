#include "Enemy.h"
#include "BossBrainComponent.h"
#include "BossAnimationComponent.h"
#include "BossEffects.h"
#include "BossSpriteAtlas.h"
#include "ParticleCatalog.h"
#include "CharacterFactory.h"
#include "GameSettings.h"
#include "GameResources.h"
#include "WeaponSetup.h"
#include "EnemyAttackComponent.h"
#include "HealthBarComponent.h"
#include "BloodPool.h"
#include "LootDrop.h"
#include "Fx.h"
#include <GameWorld.h>
#include "ChaseComponent.h"
#include "PatrolComponent.h"
#include "PatrolRoutes.h"
#include "WeaponComponent.h"
#include "MeleeWeaponComponent.h"
#include "SettleComponent.h"
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateEnemy(const EnemyConfig& config, const XYZEngine::Vector2Df& position,
        const BossDefinition* definition)
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
        spec.hasWeaponLayer = definition == nullptr || HasWeaponLayer(*definition);

        ChaseComponent* chase = nullptr;
        CharacterParts parts = CreateCharacter(spec, [&chase, &config, &position, definition](XYZEngine::GameObject* object)
        {
            chase = object->AddComponent<ChaseComponent>();
            chase->SetTargetName(PLAYER_OBJECT_NAME);
            chase->SetDetectionRadius(config.detectionRadius);
            chase->SetStopDistance(config.stopDistance);
            chase->SetAlertTime(config.alertTime);
            chase->SetVisionHalfAngle(config.visionHalfAngle);
            chase->SetAlertHalfAngle(config.alertHalfAngle);
            chase->SetSearchTime(config.searchTime);
            chase->SetLook(config.lookTime, config.lookHalfSweep);
            chase->SetSearchSpots(config.searchRadius, config.searchSpotsMin, config.searchSpotsMax);
            chase->SetSearchLook(config.searchLookTime);
            chase->SetAlertRadiusScale(config.alertRadiusScale);

            if (definition != nullptr)
            {
                return;
            }

            auto patrol = object->AddComponent<PatrolComponent>();
            patrol->SetLook(config.lookTime, config.lookHalfSweep);
            const PatrolRoute* route = PatrolRoutes::Current().Nearest(position, PATROL_JOIN_DISTANCE);
            if (route != nullptr)
            {
                patrol->SetPoints(route->points);
            }
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

        aim->SetMaxDistance(0.f);

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
            Fx::SpawnBloodHit(damage.source.position, damage.source.direction);

            if (meleeComponent != nullptr)
            {
                meleeComponent->CancelAttack();
            }

            animation->PlayHurt();
            hurtAudio->Play();
            hitFlash->Flash();
        });

        auto settle = gameObject->AddComponent<SettleComponent>();
        settle->SetEnabled(false);
        settle->SetReadyCheck([animation]()
        {
            return animation == nullptr
                || (animation->GetCurrentAnimation() == XYZEngine::MovementAnimation::Death && animation->IsFinished());
        });

        auto weaponComponent = gameObject->GetComponent<WeaponComponent>();
        const char* lootTable = config.lootTable;
        health->SubscribeDeath([gameObject, animation, movement, chase, collider, aim, weaponComponent, meleeComponent, lootTable, settle](const DeathInfo& death)
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

            if (lootTable != nullptr)
            {
                DropLoot(lootTable, gameObject, death.position);
            }

            settle->SetEnabled(true);
        });

        gameObject->SetRenderLayer(ENEMY_RENDER_LAYER);

        LOG_INFO(std::string(config.objectName) + " created at " + std::to_string(static_cast<int>(position.x)) + ";" + std::to_string(static_cast<int>(position.y)));
        return gameObject;
    }

    XYZEngine::GameObject* CreateBoss(const EnemyConfig& config, const BossDefinition& definition, const XYZEngine::Vector2Df& position)
    {
        auto gameObject = CreateEnemy(config, position, &definition);

        auto healthBar = gameObject->GetComponent<HealthBarComponent>();
        if (healthBar != nullptr)
        {
            healthBar->SetSize(BOSS_HEALTH_BAR_WIDTH, BOSS_HEALTH_BAR_HEIGHT);
            healthBar->SetOffset(0.f, BOSS_HEALTH_BAR_OFFSET_Y);
            healthBar->SetColors(BOSS_HEALTH_BAR_COLOR, VITALS_HUD_BACK_COLOR);
            healthBar->SetAlwaysVisible(true);
        }

        BossAnimationComponent* bossAnimation = nullptr;
        if (definition.textureMapName != nullptr)
        {
            auto renderer = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();
            if (renderer != nullptr)
            {
                renderer->SetPixelSize(definition.frameSize, definition.frameSize);
            }

            auto sharedAnimation = gameObject->GetComponent<XYZEngine::SpriteMovementAnimationComponent>();
            if (sharedAnimation != nullptr)
            {
                sharedAnimation->SetEnabled(false);
            }

            bossAnimation = gameObject->AddComponent<BossAnimationComponent>();
            bossAnimation->SetTextureMap(definition.textureMapName);
            bossAnimation->SetAnimation(BOSS_SLOT_IDLE, PUPPETEER_IDLE_ANIMATION);
            bossAnimation->SetAnimation(BOSS_SLOT_GLIDE, PUPPETEER_GLIDE_ANIMATION);
            bossAnimation->SetAnimation(BOSS_SLOT_SUMMON, PUPPETEER_SUMMON_ANIMATION);
            bossAnimation->SetAnimation(BOSS_SLOT_CURSE, PUPPETEER_CURSE_ANIMATION);
            bossAnimation->SetAnimation(BOSS_SLOT_HURT, PUPPETEER_HURT_ANIMATION);
            bossAnimation->SetAnimation(BOSS_SLOT_DEATH, PUPPETEER_DEATH_ANIMATION);
            bossAnimation->SetAnimation(BOSS_SLOT_ENRAGE, PUPPETEER_ENRAGE_ANIMATION);
        }

        auto brain = gameObject->AddComponent<BossBrainComponent>();
        brain->SetDefinition(&definition);
        brain->SetConfig(config);
        brain->SetTargetName(PLAYER_OBJECT_NAME);
        brain->SetBasicAttack(gameObject->GetComponent<EnemyAttackComponent>());

        auto rageAura = gameObject->AddComponent<XYZEngine::ParticleAuraComponent>();
        rageAura->SetSpec(FindParticleSpec(ParticleEffect::RageAura));
        rageAura->SetRadius(RAGE_AURA_RADIUS);

        SpawnProjectilesOnBossVolley(brain, gameObject, config.weapon);
        SummonMinionsOnBossCall(brain);
        PlayEffectsOnBossBlast(brain);
        ShowMarkOnBossCast(brain);
        PlayEffectsOnBossRage(brain, gameObject->GetComponent<HitFlashComponent>(), healthBar, rageAura);

        if (bossAnimation != nullptr)
        {
            PlayBossAnimations(brain, bossAnimation, gameObject->GetComponent<HealthComponent>());
        }

        LOG_INFO(std::string("Boss ") + definition.id + " created with health " + std::to_string(static_cast<int>(config.maxHealth)));
        return gameObject;
    }
}
