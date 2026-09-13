#include "Player.h"
#include "CharacterFactory.h"
#include "GameSettings.h"
#include "PlayerAttackComponent.h"
#include "PlayerLoadoutComponent.h"
#include "PlayerRollComponent.h"
#include "StowedWeaponComponent.h"
#include "BloodPool.h"
#include "InteractionComponent.h"
#include "InventoryComponent.h"
#include "ItemEffectComponent.h"
#include "StaminaComponent.h"
#include "Fx.h"
#include <FrameClock.h>
#include "WeaponSetup.h"
#include <GameWorld.h>
#include <RenderSystem.h>
#include <CameraComponent.h>
#include <InputComponent.h>
#include "WeaponComponent.h"
#include "MeleeWeaponComponent.h"
#include "AmmoPouchComponent.h"
#include "DodgeRollComponent.h"
#include <LoggerRegistry.h>
#include <ResourceSystem.h>

namespace RoguelikeGame
{
    namespace
    {
        StowedWeaponComponent* CreateStowedWeapon(XYZEngine::GameObject* owner, WeaponId startWeapon,
                                                  XYZEngine::SpriteMovementAnimationComponent* animation)
        {
            auto stowedObject = XYZEngine::GameWorld::Instance()->CreateGameObject("StowedWeapon", owner);

            auto stowedRenderer = AddWeaponSprite(stowedObject, startWeapon, WEAPON_STOWED_VARIANT);
            if (stowedRenderer == nullptr)
            {
                XYZEngine::GameWorld::Instance()->DestroyGameObject(stowedObject);
                return nullptr;
            }
            stowedRenderer->SetVisible(false);

            auto stowedWeapon = stowedObject->AddComponent<StowedWeaponComponent>();
            stowedWeapon->SetOwnerAnimation(animation);
            stowedWeapon->SetWeaponId(startWeapon);

            return stowedWeapon;
        }
    }

    XYZEngine::GameObject* CreatePlayer(const XYZEngine::Vector2Df& position)
    {
        WeaponId startWeapon = PLAYER_LOADOUT[PLAYER_START_WEAPON_SLOT].id;

        CharacterSpec spec;
        spec.objectName = PLAYER_OBJECT_NAME;
        spec.textureMapName = PLAYER_TEXTURE;
        spec.position = position;
        spec.collisionLayer = PLAYER_COLLISION_LAYER;
        spec.speed = PLAYER_SPEED;
        spec.maxHealth = PLAYER_MAX_HEALTH;
        spec.armor = PLAYER_ARMOR;
        spec.faction = Faction::Player;
        spec.weapon = startWeapon;
        spec.healthBarColor = {90, 200, 90};

        CharacterParts parts = CreateCharacter(spec, [](XYZEngine::GameObject* object)
        {
            auto camera = object->AddComponent<XYZEngine::CameraComponent>();
            camera->SetViewHeight(CAMERA_VIEW_HEIGHT);
            camera->SetMaxShakeAmplitude(CAMERA_SHAKE_LIMIT);
            camera->SetRotationEnabled(false);

            object->AddComponent<XYZEngine::InputComponent>();
        });

        auto gameObject = parts.gameObject;
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

        animation->SetRunAnimation(PLAYER_TEXTURE, AtlasFrameIndex(RUN_ANIMATION.row, 0), RUN_ANIMATION.frames, RUN_ANIMATION.secondsPerFrame);
        animation->SetHeavyAnimation(PLAYER_TEXTURE, AtlasFrameIndex(HEAVY_ANIMATION_ROW, 0), HEAVY_ANIMATION_FRAMES, HEAVY_FRAME_SECONDS, heavyLoops);
        animation->SetSwapAnimation(PLAYER_TEXTURE, AtlasFrameIndex(SWAP_ANIMATION_ROW, 0), SWAP_ANIMATION_FRAMES, SWAP_FRAME_SECONDS);

        int rollFirstFrames[ROLL_DIRECTIONS];
        for (int direction = 0; direction < ROLL_DIRECTIONS; direction++)
        {
            rollFirstFrames[direction] = RollFirstFrame(direction);
        }
        animation->SetRollAnimations(PLAYER_TEXTURE, rollFirstFrames, ROLL_DIRECTIONS, ROLL_ANIMATION_FRAMES, ROLL_FRAME_SECONDS);

        auto dodgeRoll = gameObject->AddComponent<DodgeRollComponent>();
        dodgeRoll->SetSpeeds(ROLL_MOVE_SPEED, ROLL_ANIMATION_FRAMES, PLAYER_ROLL_SPEED);
        dodgeRoll->SetMaxStep(PLAYER_ROLL_MAX_STEP);
        dodgeRoll->SetInvulnerableFrames(ROLL_INVULNERABLE_FIRST_FRAME, ROLL_INVULNERABLE_LAST_FRAME);
        dodgeRoll->SetIgnoredLayers(ENEMY_COLLISION_LAYER);
        dodgeRoll->SetCooldown(PLAYER_ROLL_COOLDOWN);

        auto ammoPouch = gameObject->AddComponent<AmmoPouchComponent>();
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

        StowedWeaponComponent* stowedWeapon = CreateStowedWeapon(gameObject, startWeapon, animation);

        auto weaponComponent = gameObject->AddComponent<WeaponComponent>();
        PlayEffectsOnReload(weaponComponent, animation, reloadAudio);
        PlayEffectsOnShot(weaponComponent, shotAudio, animation, parts.weapon);
        SpawnProjectilesOnShot(weaponComponent);

        auto meleeWeapon = gameObject->AddComponent<MeleeWeaponComponent>();
        PlayEffectsOnMeleeHit(meleeWeapon, meleeAudio);
        meleeWeapon->SubscribeStrike([](MeleeAttackKind kind, int hits)
        {
            if (hits <= 0)
            {
                return;
            }

            bool isHeavy = kind == MeleeAttackKind::Heavy;
            Fx::ShakeCamera(isHeavy ? CAMERA_SHAKE_HEAVY : CAMERA_SHAKE_LIGHT);
            XYZEngine::FrameClock::Instance()->HitStop(isHeavy ? HIT_STOP_HEAVY : HIT_STOP_LIGHT);
        });

        auto loadout = gameObject->AddComponent<PlayerLoadoutComponent>();
        loadout->SetWeapon(parts.weapon);
        loadout->SetStowedWeapon(stowedWeapon);
        loadout->SetAudio(shotAudio, reloadAudio);
        loadout->SetSlots(PLAYER_LOADOUT, PLAYER_WEAPON_SLOTS, PLAYER_START_WEAPON_SLOT);

        gameObject->AddComponent<InventoryComponent>()->SetCapacity(INVENTORY_CAPACITY);
        gameObject->AddComponent<InteractionComponent>();

        auto effects = gameObject->AddComponent<ItemEffectComponent>();
        effects->SetHandler(ItemEffectKind::Heal, [health](const ItemEffect& effect)
        {
            return health->Heal(effect.amount) > 0.f;
        });
        effects->SetHandler(ItemEffectKind::AddAmmo, [ammoPouch](const ItemEffect& effect)
        {
            AmmoKind kind = AmmoKind::None;
            if (!TryGetAmmoKind(effect.target, kind) || effect.amount <= 0.f)
            {
                return false;
            }

            ammoPouch->AddAmmo(static_cast<int>(kind), static_cast<int>(effect.amount));
            return true;
        });
        effects->SetHandler(ItemEffectKind::EquipWeapon, [loadout](const ItemEffect& effect)
        {
            WeaponId id = WeaponId::Knife;

            return TryGetWeaponId(effect.target, id) && loadout->EquipWeapon(id);
        });

        auto stamina = gameObject->AddComponent<StaminaComponent>();
        stamina->SetMaxStamina(PLAYER_MAX_STAMINA);
        stamina->SetRegen(PLAYER_STAMINA_REGEN, PLAYER_STAMINA_REGEN_DELAY);
        stamina->SetRunDrain(PLAYER_STAMINA_RUN_DRAIN);
        stamina->SetRunResumePart(PLAYER_STAMINA_RUN_RESUME_PART);

        gameObject->AddComponent<PlayerAttackComponent>();
        gameObject->AddComponent<PlayerRollComponent>();

        health->SubscribeHeal([gameObject](float restored)
        {
            Fx::SpawnHealBurst(gameObject->GetTransform()->GetWorldPosition());
        });

        health->SubscribeDamage([animation, hurtAudio, hitFlash, meleeWeapon](const DamageInfo& damage)
        {
            Fx::ShakeCamera(CAMERA_SHAKE_HEAVY);
            meleeWeapon->CancelAttack();
            animation->PlayHurt();
            hurtAudio->Play();
            hitFlash->Flash();
        });

        health->SubscribeDeath([gameObject, animation, movement, collider, aim, weaponComponent, meleeWeapon, dodgeRoll, reloadAudio, hitFlash](const DeathInfo& death)
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

            gameObject->SetRenderLayer(CORPSE_RENDER_LAYER);
            BloodPool::Spawn(death.position, death.rotation);
            XYZEngine::FrameClock::Instance()->SlowMotion(DEATH_TIME_SCALE, DEATH_SLOW_MOTION_TIME, DEATH_SLOW_MOTION_BLEND);

            LOG_WARN("Player is dead, controls are disabled");
        });

        gameObject->SetRenderLayer(PLAYER_RENDER_LAYER);

        if (stowedWeapon != nullptr)
        {
            stowedWeapon->GetGameObject()->SetRenderLayer(STOWED_WEAPON_RENDER_LAYER);
        }

        LOG_INFO("Player created at " + std::to_string(static_cast<int>(position.x)) + ";" + std::to_string(static_cast<int>(position.y)));
        return gameObject;
    }
}
