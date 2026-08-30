#include "Enemy.h"
#include "GameSettings.h"
#include "Projectile.h"
#include "EnemyAttackComponent.h"
#include "HitFlashComponent.h"
#include "BloodPool.h"
#include <GameWorld.h>
#include <SpriteRendererComponent.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <ChaseComponent.h>
#include <MovementComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>
#include <AimRotationComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include <HealthComponent.h>
#include <HealthBarComponent.h>
#include <WeaponComponent.h>
#include <AudioComponent.h>
#include <stdexcept>

namespace RoguelikeGame
{
    Enemy::Enemy(const EnemyConfig& config, const XYZEngine::Vector2Df& position)
    {
        gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(config.objectName);

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetWorldPosition(position);

        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(config.textureMapName,
                                                                                         AtlasFrameIndex(IDLE_ANIMATION.row, 0));
        if (texture == nullptr)
        {
            XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
            throw std::runtime_error("enemy texture map is not loaded: " + config.textureMapName);
        }

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(CHARACTER_SPRITE_SIZE, CHARACTER_SPRITE_SIZE);

        // Chase задает направление, а movement реализует его. Сначала обновляй Chase.
        auto chase = gameObject->AddComponent<XYZEngine::ChaseComponent>();
        chase->SetTargetName("Player");
        chase->SetDetectionRadius(config.detectionRadius);
        chase->SetStopDistance(config.stopDistance);

        auto movement = gameObject->AddComponent<XYZEngine::MovementComponent>();
        movement->SetSpeed(config.speed);

        auto body = gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
        body->SetKinematic(false);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(CHARACTER_COLLIDER_SIZE, CHARACTER_COLLIDER_SIZE);

        auto aim = gameObject->AddComponent<XYZEngine::AimRotationComponent>();
        aim->AimAtGameObject("Player");
        aim->SetMaxDistance(config.detectionRadius);

        auto animation = gameObject->AddComponent<XYZEngine::SpriteMovementAnimationComponent>();
        animation->SetIdleAnimation(config.textureMapName, AtlasFrameIndex(IDLE_ANIMATION.row, 0), IDLE_ANIMATION.frames, IDLE_ANIMATION.framesPerSecond);
        animation->SetWalkAnimation(config.textureMapName, AtlasFrameIndex(WALK_ANIMATION.row, 0), WALK_ANIMATION.frames, WALK_ANIMATION.framesPerSecond);
        animation->SetShootAnimation(config.textureMapName, AtlasFrameIndex(SHOOT_ANIMATION.row, 0), SHOOT_ANIMATION.frames, SHOOT_ANIMATION.framesPerSecond);
        animation->SetReloadAnimation(config.textureMapName, AtlasFrameIndex(RELOAD_ANIMATION.row, 0), RELOAD_ANIMATION.frames,
                                      ReloadFramesPerSecond(GetWeapon(config.weapon).reloadTime));
        animation->SetHurtAnimation(config.textureMapName, AtlasFrameIndex(HURT_ANIMATION.row, 0), HURT_ANIMATION.frames, HURT_ANIMATION.framesPerSecond);
        animation->SetDeathAnimation(config.textureMapName, AtlasFrameIndex(DEATH_ANIMATION.row, 0), DEATH_ANIMATION.frames, DEATH_ANIMATION.framesPerSecond);

        auto health = gameObject->AddComponent<XYZEngine::HealthComponent>();
        health->SetMaxHealth(config.maxHealth);
        health->SetArmor(config.armor);

        auto healthBar = gameObject->AddComponent<XYZEngine::HealthBarComponent>();
        healthBar->SetSize(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT);
        healthBar->SetOffset(0.f, HEALTH_BAR_OFFSET_Y);
        healthBar->SetColors({200, 60, 60}, {20, 20, 20, 200});

        auto hurtAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        hurtAudio->SetSound(XYZEngine::ResourceSystem::Instance()->GetSound(HURT_SOUND));
        hurtAudio->SetVolume(HURT_VOLUME);

        auto hitFlash = gameObject->AddComponent<HitFlashComponent>();
        hitFlash->AddRenderer(renderer);

        try
        {
            weapon = std::make_unique<Weapon>(gameObject, config.weapon, animation);
            hitFlash->AddRenderer(weapon->GetRenderer());
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Enemy weapon is not created: ") + exception.what());
        }

        if (config.attackRange <= 0.f)
        {
            LOG_INFO(config.objectName + " is unarmed and can't shoot");
        }
        else
        {
            auto shotAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
            shotAudio->SetSound(XYZEngine::ResourceSystem::Instance()->GetSound(SHOT_SOUND));
            shotAudio->SetVolume(SHOT_VOLUME);

            const WeaponDefinition& weaponDefinition = GetWeapon(config.weapon);
            auto muzzleFlash = weapon == nullptr ? nullptr : weapon->GetMuzzleFlash();

            auto weaponComponent = gameObject->AddComponent<XYZEngine::WeaponComponent>();
            weaponComponent->SetCooldown(config.attackCooldown);
            weaponComponent->SetDamage(config.attackDamage);
            weaponComponent->SetProjectileSpeed(config.projectileSpeed);
            weaponComponent->SetMuzzleOffset(ShotOffset(weaponDefinition));

            weaponComponent->SetMagazine(weaponDefinition.magazineSize, AmmoKindKey(weaponDefinition.ammo));
            weaponComponent->SetReloadTime(weaponDefinition.reloadTime);
            weaponComponent->SetReloadStartAction([animation]() { animation->PlayReload(); });

            std::string shooterName = config.objectName;
            BulletKind bullet = weaponDefinition.bullet;
            weaponComponent->SetShotAction(
                [shooterName, bullet, shotAudio, animation, muzzleFlash](const XYZEngine::Vector2Df& shotPosition, const XYZEngine::Vector2Df& shotDirection,
                                                                         float damage, float speed)
                {
                    Projectile::Spawn(shotPosition, shotDirection, damage, speed, shooterName, bullet);
                    shotAudio->Play();
                    animation->PlayShoot();

                    if (muzzleFlash != nullptr)
                    {
                        muzzleFlash->Play();
                    }
                });

            auto attack = gameObject->AddComponent<EnemyAttackComponent>();
            attack->SetTargetName("Player");
            attack->SetAttackRange(config.attackRange);
        }

        health->SubscribeDamage([animation, hurtAudio, hitFlash](float damage)
        {
            animation->PlayHurt();
            hurtAudio->Play();
            hitFlash->Flash();
        });

        auto characterObject = gameObject;
        auto weaponComponent = gameObject->GetComponent<XYZEngine::WeaponComponent>();
        health->SubscribeDeath([characterObject, transform, animation, movement, chase, collider, aim, weaponComponent]()
        {
            if (weaponComponent != nullptr)
            {
                weaponComponent->CancelReload();
            }

            animation->PlayDeath();
            movement->SetSpeed(0.f);
            chase->SetDetectionRadius(0.f);
            collider->SetTrigger(true);
            aim->SetEnabled(false);

            characterObject->SetRenderLayer(CORPSE_RENDER_LAYER);
            BloodPool::Spawn(transform->GetWorldPosition(), transform->GetWorldRotation());
        });

        gameObject->SetRenderLayer(ENEMY_RENDER_LAYER);

        LOG_INFO(config.objectName + " created at " + std::to_string(static_cast<int>(position.x)) + ";" + std::to_string(static_cast<int>(position.y)));
    }

    XYZEngine::GameObject* Enemy::GetGameObject()
    {
        return gameObject;
    }
}
