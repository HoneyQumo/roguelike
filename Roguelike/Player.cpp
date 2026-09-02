#include "Player.h"
#include "GameSettings.h"
#include "GameResources.h"
#include "PlayerAttackComponent.h"
#include "PlayerLoadoutComponent.h"
#include "PlayerRollComponent.h"
#include "StowedWeaponComponent.h"
#include "HitFlashComponent.h"
#include "BloodPool.h"
#include <GameWorld.h>
#include <RenderSystem.h>
#include <CameraComponent.h>
#include <InputComponent.h>
#include <SpriteRendererComponent.h>
#include <WeaponComponent.h>
#include <MeleeWeaponComponent.h>
#include <AmmoPouchComponent.h>
#include <AudioComponent.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <MovementComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>
#include <AimRotationComponent.h>
#include <DodgeRollComponent.h>
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
        collider->SetCollisionLayer(PLAYER_COLLISION_LAYER);

        auto aim = gameObject->AddComponent<XYZEngine::AimRotationComponent>();
        aim->AimAtCursor();

        WeaponId startWeapon = PLAYER_LOADOUT[PLAYER_START_WEAPON_SLOT];

        XYZEngine::ChargedAnimationLoops heavyLoops;
        heavyLoops.chargeFirstFrame = HEAVY_CHARGE_LOOP_FIRST;
        heavyLoops.chargeLastFrame = HEAVY_CHARGE_LOOP_LAST;
        heavyLoops.chargedFirstFrame = HEAVY_CHARGED_LOOP_FIRST;
        heavyLoops.chargedLastFrame = HEAVY_CHARGED_LOOP_LAST;
        heavyLoops.releaseFrame = HEAVY_RELEASE_FRAME;

        auto animation = gameObject->AddComponent<XYZEngine::SpriteMovementAnimationComponent>();
        animation->SetIdleAnimation(PLAYER_TEXTURE, AtlasFrameIndex(IDLE_ANIMATION.row, 0), IDLE_ANIMATION.frames, IDLE_ANIMATION.framesPerSecond);
        animation->SetWalkAnimation(PLAYER_TEXTURE, AtlasFrameIndex(WALK_ANIMATION.row, 0), WALK_ANIMATION.frames, WALK_ANIMATION.framesPerSecond);
        animation->SetRunAnimation(PLAYER_TEXTURE, AtlasFrameIndex(RUN_ANIMATION.row, 0), RUN_ANIMATION.frames, RUN_ANIMATION.framesPerSecond);
        animation->SetShootAnimation(PLAYER_TEXTURE, AtlasFrameIndex(SHOOT_ANIMATION.row, 0), SHOOT_ANIMATION.frames, SHOOT_ANIMATION.framesPerSecond);
        animation->SetReloadAnimation(PLAYER_TEXTURE, AtlasFrameIndex(RELOAD_ANIMATION.row, 0), RELOAD_ANIMATION.frames,
                                      ReloadFramesPerSecond(GetWeapon(startWeapon).reloadTime));
        animation->SetMeleeAnimation(PLAYER_TEXTURE, AtlasFrameIndex(MELEE_ANIMATION.row, 0), MELEE_ANIMATION.frames, MELEE_ANIMATION.framesPerSecond);
        animation->SetHeavyAnimation(PLAYER_TEXTURE, AtlasFrameIndex(HEAVY_ANIMATION_ROW, 0), HEAVY_ANIMATION_FRAMES, HEAVY_FRAME_SECONDS, heavyLoops);
        animation->SetSwapAnimation(PLAYER_TEXTURE, AtlasFrameIndex(SWAP_ANIMATION_ROW, 0), SWAP_ANIMATION_FRAMES, SWAP_FRAME_SECONDS);
        animation->SetHurtAnimation(PLAYER_TEXTURE, AtlasFrameIndex(HURT_ANIMATION.row, 0), HURT_ANIMATION.frames, HURT_ANIMATION.framesPerSecond);
        animation->SetDeathAnimation(PLAYER_TEXTURE, AtlasFrameIndex(DEATH_ANIMATION.row, 0), DEATH_ANIMATION.frames, DEATH_ANIMATION.framesPerSecond);

        int rollFirstFrames[ROLL_DIRECTIONS];
        for (int direction = 0; direction < ROLL_DIRECTIONS; direction++)
        {
            rollFirstFrames[direction] = RollFirstFrame(direction);
        }
        animation->SetRollAnimations(PLAYER_TEXTURE, rollFirstFrames, ROLL_DIRECTIONS, ROLL_ANIMATION_FRAMES, ROLL_FRAMES_PER_SECOND);

        auto dodgeRoll = gameObject->AddComponent<XYZEngine::DodgeRollComponent>();
        dodgeRoll->SetSpeeds(ROLL_MOVE_SPEED, ROLL_ANIMATION_FRAMES, PLAYER_ROLL_SPEED);
        dodgeRoll->SetMaxStep(PLAYER_ROLL_MAX_STEP);
        dodgeRoll->SetInvulnerableFrames(ROLL_INVULNERABLE_FIRST_FRAME, ROLL_INVULNERABLE_LAST_FRAME);
        dodgeRoll->SetIgnoredLayers(ENEMY_COLLISION_LAYER);
        dodgeRoll->SetCooldown(PLAYER_ROLL_COOLDOWN);

        auto health = gameObject->AddComponent<XYZEngine::HealthComponent>();
        health->SetMaxHealth(PLAYER_MAX_HEALTH);
        health->SetArmor(PLAYER_ARMOR);

        auto ammoPouch = gameObject->AddComponent<XYZEngine::AmmoPouchComponent>();
        for (const AmmoReserve& reserve : PLAYER_START_AMMO)
        {
            ammoPouch->SetAmmo(AmmoKindKey(reserve.kind), reserve.count);
        }

        auto shotAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        shotAudio->SetVolume(SHOT_VOLUME);

        auto reloadAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        reloadAudio->SetVolume(RELOAD_VOLUME);

        auto meleeAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        meleeAudio->SetVolume(MELEE_HIT_VOLUME);

        auto hurtAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        hurtAudio->SetSound(XYZEngine::ResourceSystem::Instance()->GetSound(HURT_SOUND));
        hurtAudio->SetVolume(HURT_VOLUME);

        auto hitFlash = gameObject->AddComponent<HitFlashComponent>();
        hitFlash->AddRenderer(renderer);

        try
        {
            weapon = std::make_unique<Weapon>(gameObject, startWeapon, animation);
            hitFlash->AddRenderer(weapon->GetRenderer());
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Player weapon is not created: ") + exception.what());
        }

        StowedWeaponComponent* stowedWeapon = CreateStowedWeapon(startWeapon, animation);

        auto weaponComponent = gameObject->AddComponent<XYZEngine::WeaponComponent>();
        weaponComponent->SetReloadStartAction([animation, reloadAudio]()
        {
            animation->PlayReload();
            reloadAudio->Play();
        });

        auto meleeWeapon = gameObject->AddComponent<XYZEngine::MeleeWeaponComponent>();

        auto loadout = gameObject->AddComponent<PlayerLoadoutComponent>();
        loadout->SetWeapon(weapon.get());
        loadout->SetStowedWeapon(stowedWeapon);
        loadout->SetAudio(shotAudio, reloadAudio, meleeAudio);
        loadout->SetSlots(PLAYER_LOADOUT, PLAYER_WEAPON_SLOTS, PLAYER_START_WEAPON_SLOT);

        gameObject->AddComponent<PlayerAttackComponent>();
        gameObject->AddComponent<PlayerRollComponent>();

        health->SubscribeDamage([animation, hurtAudio, hitFlash, meleeWeapon](float damage)
        {
            meleeWeapon->CancelAttack();
            animation->PlayHurt();
            hurtAudio->Play();
            hitFlash->Flash();
        });

        auto characterObject = gameObject;
        health->SubscribeDeath([characterObject, transform, animation, movement, collider, aim, weaponComponent, meleeWeapon, dodgeRoll, reloadAudio, hitFlash]()
        {
            dodgeRoll->CancelRoll();
            weaponComponent->CancelReload();
            meleeWeapon->CancelAttack();
            reloadAudio->Stop();
            hitFlash->SetGlow(0.f);
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

        if (stowedWeapon != nullptr)
        {
            stowedWeapon->GetGameObject()->SetRenderLayer(STOWED_WEAPON_RENDER_LAYER);
        }

        LOG_INFO("Player created at " + std::to_string(static_cast<int>(position.x)) + ";" + std::to_string(static_cast<int>(position.y)));
    }

    XYZEngine::GameObject* Player::GetGameObject()
    {
        return gameObject;
    }

    StowedWeaponComponent* Player::CreateStowedWeapon(WeaponId startWeapon, XYZEngine::SpriteMovementAnimationComponent* animation)
    {
        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(WEAPONS_TEXTURE,
                                                                                         WeaponFrameIndex(startWeapon, WEAPON_STOWED_VARIANT));
        if (texture == nullptr)
        {
            LOG_ERROR("Stowed weapon texture is not loaded");
            return nullptr;
        }

        auto stowedObject = XYZEngine::GameWorld::Instance()->CreateGameObject("StowedWeapon");

        auto stowedTransform = stowedObject->GetComponent<XYZEngine::TransformComponent>();
        stowedTransform->SetParent(gameObject->GetComponent<XYZEngine::TransformComponent>());
        stowedTransform->SetLocalPosition(0.f, 0.f);

        auto stowedRenderer = stowedObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        stowedRenderer->SetTexture(*texture);
        stowedRenderer->SetPixelSize(WEAPON_FRAME_WIDTH, WEAPON_FRAME_HEIGHT);
        stowedRenderer->SetVisible(false);

        auto stowedWeapon = stowedObject->AddComponent<StowedWeaponComponent>();
        stowedWeapon->SetOwnerAnimation(animation);
        stowedWeapon->SetWeaponId(startWeapon);

        return stowedWeapon;
    }
}
