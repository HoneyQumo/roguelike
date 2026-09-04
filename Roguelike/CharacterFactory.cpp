#include "CharacterFactory.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <RigidbodyComponent.h>
#include "HealthBarComponent.h"
#include <LoggerRegistry.h>
#include <stdexcept>

namespace RoguelikeGame
{
    CharacterParts CreateCharacter(const CharacterSpec& spec, const AddControls& addControls)
    {
        CharacterParts parts;
        parts.gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(spec.objectName);

        parts.transform = parts.gameObject->GetTransform();
        parts.transform->SetWorldPosition(spec.position);

        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(spec.textureMapName,
                                                                                         AtlasFrameIndex(IDLE_ANIMATION.row, 0));
        if (texture == nullptr)
        {
            XYZEngine::GameWorld::Instance()->DestroyGameObject(parts.gameObject);
            throw std::runtime_error("character texture map is not loaded: " + spec.textureMapName);
        }

        parts.renderer = parts.gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        parts.renderer->SetTexture(*texture);
        parts.renderer->SetPixelSize(CHARACTER_SPRITE_SIZE, CHARACTER_SPRITE_SIZE);

        if (addControls != nullptr)
        {
            addControls(parts.gameObject);
        }

        parts.movement = parts.gameObject->AddComponent<XYZEngine::MovementComponent>();
        parts.movement->SetSpeed(spec.speed);

        auto body = parts.gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
        body->SetKinematic(false);

        parts.collider = parts.gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        parts.collider->SetSize(CHARACTER_COLLIDER_SIZE, CHARACTER_COLLIDER_SIZE);
        parts.collider->SetCollisionLayer(spec.collisionLayer);

        parts.aim = parts.gameObject->AddComponent<XYZEngine::AimRotationComponent>();

        const std::string& atlas = spec.textureMapName;
        parts.animation = parts.gameObject->AddComponent<XYZEngine::SpriteMovementAnimationComponent>();
        parts.animation->SetIdleAnimation(atlas, AtlasFrameIndex(IDLE_ANIMATION.row, 0), IDLE_ANIMATION.frames, IDLE_ANIMATION.secondsPerFrame);
        parts.animation->SetWalkAnimation(atlas, AtlasFrameIndex(WALK_ANIMATION.row, 0), WALK_ANIMATION.frames, WALK_ANIMATION.secondsPerFrame);
        parts.animation->SetShootAnimation(atlas, AtlasFrameIndex(SHOOT_ANIMATION.row, 0), SHOOT_ANIMATION.frames, SHOOT_ANIMATION.secondsPerFrame);
        parts.animation->SetReloadAnimation(atlas, AtlasFrameIndex(RELOAD_ANIMATION.row, 0), RELOAD_ANIMATION.frames,
                                            ReloadFrameSeconds(GetWeapon(spec.weapon).reloadTime));
        parts.animation->SetMeleeAnimation(atlas, AtlasFrameIndex(MELEE_ANIMATION.row, 0), MELEE_ANIMATION.frames, MELEE_ANIMATION.secondsPerFrame);
        parts.animation->SetHurtAnimation(atlas, AtlasFrameIndex(HURT_ANIMATION.row, 0), HURT_ANIMATION.frames, HURT_ANIMATION.secondsPerFrame);
        parts.animation->SetDeathAnimation(atlas, AtlasFrameIndex(DEATH_ANIMATION.row, 0), DEATH_ANIMATION.frames, DEATH_ANIMATION.secondsPerFrame);

        parts.health = parts.gameObject->AddComponent<HealthComponent>();
        parts.health->SetMaxHealth(spec.maxHealth);
        parts.health->SetArmor(spec.armor);

        auto healthBar = parts.gameObject->AddComponent<HealthBarComponent>();
        healthBar->SetSize(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT);
        healthBar->SetOffset(0.f, HEALTH_BAR_OFFSET_Y);
        healthBar->SetColors(spec.healthBarColor, {20, 20, 20, 200});

        parts.hurtAudio = parts.gameObject->AddComponent<XYZEngine::AudioComponent>();
        parts.hurtAudio->SetSound(XYZEngine::ResourceSystem::Instance()->GetSound(HURT_SOUND));
        parts.hurtAudio->SetVolume(HURT_VOLUME);

        parts.hitFlash = parts.gameObject->AddComponent<HitFlashComponent>();
        parts.hitFlash->AddRenderer(parts.renderer);

        try
        {
            parts.weapon = CreateWeapon(parts.gameObject, spec.weapon, parts.animation);
            parts.hitFlash->AddRenderer(parts.weapon->GetGameObject()->GetComponent<XYZEngine::SpriteRendererComponent>());
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(spec.objectName + " weapon is not created: " + exception.what());
        }

        return parts;
    }
}
