#include "Player.h"
#include "GameSettings.h"
#include "Projectile.h"
#include "PlayerAttackComponent.h"
#include "HitFlashComponent.h"
#include "BloodPool.h"
#include <GameWorld.h>
#include <RenderSystem.h>
#include <CameraComponent.h>
#include <InputComponent.h>
#include <SpriteRendererComponent.h>
#include <WeaponComponent.h>
#include <AmmoPouchComponent.h>
#include <AudioComponent.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <MovementComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>
#include <AimRotationComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include <HealthComponent.h>
#include <HealthBarComponent.h>
#include <stdexcept>

namespace RoguelikeGame
{
    Player::Player(const XYZEngine::Vector2Df& position)
    {
        gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Player");

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetWorldPosition(position);

        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(PLAYER_TEXTURE,
                                                                                         AtlasFrameIndex(IDLE_ANIMATION.row, 0));
        if (texture == nullptr)
        {
            XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
            throw std::runtime_error("player texture map is not loaded");
        }

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(CHARACTER_SPRITE_SIZE, CHARACTER_SPRITE_SIZE);

        auto camera = gameObject->AddComponent<XYZEngine::CameraComponent>();
        camera->SetWindow(&XYZEngine::RenderSystem::Instance()->GetMainWindow());
        camera->SetBaseResolution(SCREEN_WIDTH, SCREEN_HEIGHT);
        camera->SetRotationEnabled(false);

        auto input = gameObject->AddComponent<XYZEngine::InputComponent>();

        auto movement = gameObject->AddComponent<XYZEngine::MovementComponent>();
        movement->SetSpeed(PLAYER_SPEED);
        movement->SetRunSpeedMultiplier(PLAYER_RUN_SPEED_MULTIPLIER);

        auto body = gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
        body->SetKinematic(false);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(CHARACTER_COLLIDER_SIZE, CHARACTER_COLLIDER_SIZE);

        auto aim = gameObject->AddComponent<XYZEngine::AimRotationComponent>();
        aim->AimAtCursor();

        auto animation = gameObject->AddComponent<XYZEngine::SpriteMovementAnimationComponent>();
        animation->SetIdleAnimation(PLAYER_TEXTURE, AtlasFrameIndex(IDLE_ANIMATION.row, 0), IDLE_ANIMATION.frames, IDLE_ANIMATION.framesPerSecond);
        animation->SetWalkAnimation(PLAYER_TEXTURE, AtlasFrameIndex(WALK_ANIMATION.row, 0), WALK_ANIMATION.frames, WALK_ANIMATION.framesPerSecond);
        animation->SetRunAnimation(PLAYER_TEXTURE, AtlasFrameIndex(RUN_ANIMATION.row, 0), RUN_ANIMATION.frames, RUN_ANIMATION.framesPerSecond);
        animation->SetShootAnimation(PLAYER_TEXTURE, AtlasFrameIndex(SHOOT_ANIMATION.row, 0), SHOOT_ANIMATION.frames, SHOOT_ANIMATION.framesPerSecond);
        animation->SetReloadAnimation(PLAYER_TEXTURE, AtlasFrameIndex(RELOAD_ANIMATION.row, 0), RELOAD_ANIMATION.frames,
                                      ReloadFramesPerSecond(GetWeapon(PLAYER_WEAPON).reloadTime));
        animation->SetHurtAnimation(PLAYER_TEXTURE, AtlasFrameIndex(HURT_ANIMATION.row, 0), HURT_ANIMATION.frames, HURT_ANIMATION.framesPerSecond);
        animation->SetDeathAnimation(PLAYER_TEXTURE, AtlasFrameIndex(DEATH_ANIMATION.row, 0), DEATH_ANIMATION.frames, DEATH_ANIMATION.framesPerSecond);

        auto health = gameObject->AddComponent<XYZEngine::HealthComponent>();
        health->SetMaxHealth(PLAYER_MAX_HEALTH);
        health->SetArmor(PLAYER_ARMOR);

        auto ammoPouch = gameObject->AddComponent<XYZEngine::AmmoPouchComponent>();
        for (const AmmoReserve& reserve : PLAYER_START_AMMO)
        {
            ammoPouch->SetAmmo(AmmoKindKey(reserve.kind), reserve.count);
        }

        auto shotAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        shotAudio->SetSound(XYZEngine::ResourceSystem::Instance()->GetSound(SHOT_SOUND));
        shotAudio->SetVolume(SHOT_VOLUME);

        auto hurtAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        hurtAudio->SetSound(XYZEngine::ResourceSystem::Instance()->GetSound(HURT_SOUND));
        hurtAudio->SetVolume(HURT_VOLUME);

        auto hitFlash = gameObject->AddComponent<HitFlashComponent>();
        hitFlash->AddRenderer(renderer);

        try
        {
            weapon = std::make_unique<Weapon>(gameObject, PLAYER_WEAPON, animation);
            hitFlash->AddRenderer(weapon->GetRenderer());
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Player weapon is not created: ") + exception.what());
        }

        const WeaponDefinition& weaponDefinition = GetWeapon(PLAYER_WEAPON);
        auto muzzleFlash = weapon == nullptr ? nullptr : weapon->GetMuzzleFlash();

        auto weaponComponent = gameObject->AddComponent<XYZEngine::WeaponComponent>();
        weaponComponent->SetCooldown(PLAYER_ATTACK_COOLDOWN);
        weaponComponent->SetDamage(PLAYER_ATTACK_DAMAGE);
        weaponComponent->SetProjectileSpeed(PLAYER_PROJECTILE_SPEED);
        weaponComponent->SetMuzzleOffset(ShotOffset(weaponDefinition));
        weaponComponent->SetMagazine(weaponDefinition.magazineSize, AmmoKindKey(weaponDefinition.ammo));
        weaponComponent->SetReloadTime(weaponDefinition.reloadTime);
        weaponComponent->SetReloadStartAction([animation]() { animation->PlayReload(); });
        weaponComponent->SetShotAction(
            [shotAudio, animation, muzzleFlash](const XYZEngine::Vector2Df& shotPosition, const XYZEngine::Vector2Df& shotDirection, float damage, float speed)
            {
                Projectile::Spawn(shotPosition, shotDirection, damage, speed, "Player", GetWeapon(PLAYER_WEAPON).bullet);
                shotAudio->Play();
                animation->PlayShoot();

                if (muzzleFlash != nullptr)
                {
                    muzzleFlash->Play();
                }
            });

        gameObject->AddComponent<PlayerAttackComponent>();

        health->SubscribeDamage([animation, hurtAudio, hitFlash](float damage)
        {
            animation->PlayHurt();
            hurtAudio->Play();
            hitFlash->Flash();
        });

        auto characterObject = gameObject;
        health->SubscribeDeath([characterObject, transform, animation, movement, collider, aim, weaponComponent]()
        {
            weaponComponent->CancelReload();
            animation->PlayDeath();
            movement->SetSpeed(0.f);
            collider->SetTrigger(true);
            aim->SetEnabled(false);

            characterObject->SetRenderLayer(CORPSE_RENDER_LAYER);
            BloodPool::Spawn(transform->GetWorldPosition(), transform->GetWorldRotation());

            LOG_WARN("Player is dead, controls are disabled");
        });

        auto healthBar = gameObject->AddComponent<XYZEngine::HealthBarComponent>();
        healthBar->SetSize(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT);
        healthBar->SetOffset(0.f, HEALTH_BAR_OFFSET_Y);
        healthBar->SetColors({90, 200, 90}, {20, 20, 20, 200});

        gameObject->SetRenderLayer(PLAYER_RENDER_LAYER);

        LOG_INFO("Player created at " + std::to_string(static_cast<int>(position.x)) + ";" + std::to_string(static_cast<int>(position.y)));
    }

    XYZEngine::GameObject* Player::GetGameObject()
    {
        return gameObject;
    }
}
