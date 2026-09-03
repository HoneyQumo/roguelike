#include "Player.h"
#include "CharacterFactory.h"
#include "GameSettings.h"
#include "PlayerAttackComponent.h"
#include "PlayerLoadoutComponent.h"
#include "PlayerRollComponent.h"
#include "StowedWeaponComponent.h"
#include "BloodPool.h"
#include <GameWorld.h>
#include <RenderSystem.h>
#include <CameraComponent.h>
#include <InputComponent.h>
#include <WeaponComponent.h>
#include <MeleeWeaponComponent.h>
#include <AmmoPouchComponent.h>
#include <DodgeRollComponent.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>

namespace RoguelikeGame
{
    Player::Player(const XYZEngine::Vector2Df& position)
    {
        WeaponId startWeapon = PLAYER_LOADOUT[PLAYER_START_WEAPON_SLOT];

        CharacterSpec spec;
        spec.objectName = "Player";
        spec.textureMapName = PLAYER_TEXTURE;
        spec.position = position;
        spec.collisionLayer = PLAYER_COLLISION_LAYER;
        spec.speed = PLAYER_SPEED;
        spec.maxHealth = PLAYER_MAX_HEALTH;
        spec.armor = PLAYER_ARMOR;
        spec.weapon = startWeapon;
        spec.healthBarColor = {90, 200, 90};

        CharacterParts parts = CreateCharacter(spec, [](XYZEngine::GameObject* object)
        {
            auto camera = object->AddComponent<XYZEngine::CameraComponent>();
            camera->SetWindow(&XYZEngine::RenderSystem::Instance()->GetMainWindow());
            camera->SetBaseResolution(SCREEN_WIDTH, SCREEN_HEIGHT);
            camera->SetRotationEnabled(false);

            object->AddComponent<XYZEngine::InputComponent>();
        });

        gameObject = parts.gameObject;

        auto transform = parts.transform;
        auto movement = parts.movement;
        auto collider = parts.collider;
        auto aim = parts.aim;
        auto animation = parts.animation;
        auto health = parts.health;
        auto hurtAudio = parts.hurtAudio;
        auto hitFlash = parts.hitFlash;

        movement->SetRunSpeedMultiplier(PLAYER_RUN_SPEED_MULTIPLIER);
        aim->AimAtCursor();

        XYZEngine::ChargedAnimationLoops heavyLoops;
        heavyLoops.chargeFirstFrame = HEAVY_CHARGE_LOOP_FIRST;
        heavyLoops.chargeLastFrame = HEAVY_CHARGE_LOOP_LAST;
        heavyLoops.chargedFirstFrame = HEAVY_CHARGED_LOOP_FIRST;
        heavyLoops.chargedLastFrame = HEAVY_CHARGED_LOOP_LAST;
        heavyLoops.releaseFrame = HEAVY_RELEASE_FRAME;

        animation->SetRunAnimation(PLAYER_TEXTURE, AtlasFrameIndex(RUN_ANIMATION.row, 0), RUN_ANIMATION.frames, RUN_ANIMATION.framesPerSecond);
        animation->SetHeavyAnimation(PLAYER_TEXTURE, AtlasFrameIndex(HEAVY_ANIMATION_ROW, 0), HEAVY_ANIMATION_FRAMES, HEAVY_FRAME_SECONDS, heavyLoops);
        animation->SetSwapAnimation(PLAYER_TEXTURE, AtlasFrameIndex(SWAP_ANIMATION_ROW, 0), SWAP_ANIMATION_FRAMES, SWAP_FRAME_SECONDS);

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

        StowedWeaponComponent* stowedWeapon = CreateStowedWeapon(startWeapon, animation);

        auto weaponComponent = gameObject->AddComponent<XYZEngine::WeaponComponent>();
        weaponComponent->SetReloadStartAction([animation, reloadAudio]()
        {
            animation->PlayReload();
            reloadAudio->Play();
        });

        auto meleeWeapon = gameObject->AddComponent<XYZEngine::MeleeWeaponComponent>();

        auto loadout = gameObject->AddComponent<PlayerLoadoutComponent>();
        loadout->SetWeapon(parts.weapon);
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
