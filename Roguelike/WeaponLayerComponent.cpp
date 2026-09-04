#include "WeaponLayerComponent.h"
#include "Fx.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <LoggerRegistry.h>
#include <algorithm>

namespace RoguelikeGame
{
    WeaponLayerComponent::WeaponLayerComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
        CreateMuzzleFlash();
    }

    void WeaponLayerComponent::Start()
    {
        renderer = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();
    }

    void WeaponLayerComponent::Update(float deltaTime)
    {

        if (ownerAnimation == nullptr)
        {
            return;
        }

        auto animation = ownerAnimation->GetCurrentAnimation();
        int frame = ownerAnimation->GetCurrentFrame();

        bool isHidden = IsWeaponHidden(animation, frame);
        if (renderer != nullptr)
        {
            renderer->SetVisible(!isHidden);
        }

        if (isHidden)
        {
            return;
        }

        ShowVariant(GetFrameVariant(animation, frame));

        FrameOffset offset = GetFrameOffset(animation, frame);

        // Отдача масштабирует только смещение выстрела: ствол тяжелее — слой отбрасывает дальше назад.
        float forward = animation == XYZEngine::MovementAnimation::Shoot ? offset.x * recoil : offset.x;
        transform->SetLocalPosition(ToWorldOffset(forward, offset.y));
        transform->SetLocalRotation(ToWorldAngle(GetFrameRotation(animation, frame)));
    }

    void WeaponLayerComponent::Render()
    {
    }

    void WeaponLayerComponent::SetOwnerAnimation(XYZEngine::SpriteMovementAnimationComponent* newOwnerAnimation)
    {
        ownerAnimation = newOwnerAnimation;
    }

    void WeaponLayerComponent::SetWeaponId(WeaponId newWeaponId)
    {
        for (int variant = 0; variant < WEAPON_VARIANTS; variant++)
        {
            variants[variant] = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(WEAPONS_TEXTURE,
                                                                                                  WeaponFrameIndex(newWeaponId, variant));
        }

        if (renderer == nullptr)
        {
            renderer = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();
        }

        currentVariant = WEAPON_DEFAULT_VARIANT;
        if (renderer != nullptr && variants[currentVariant] != nullptr)
        {
            renderer->SetTexture(*variants[currentVariant]);
        }

        const WeaponDefinition& weapon = GetWeapon(newWeaponId);
        recoil = weapon.recoil;

        if (muzzleFlashTransform == nullptr)
        {
            return;
        }

        muzzleFlashTransform->SetLocalPosition(ToWorldOffset(weapon.muzzleX, weapon.muzzleY));

        hasMuzzleFlash = weapon.flashScale > 0.f;
        float flashScale = hasMuzzleFlash ? weapon.flashScale : 1.f;
        muzzleFlashTransform->SetLocalScale(flashScale, flashScale);

        if (!hasMuzzleFlash)
        {
            muzzleFlash->Stop();
            muzzleFlashRenderer->SetVisible(false);
        }
    }

    void WeaponLayerComponent::PlayMuzzleFlash()
    {
        if (muzzleFlash != nullptr && hasMuzzleFlash)
        {
            muzzleFlash->Play();
        }
    }

    void WeaponLayerComponent::CreateMuzzleFlash()
    {
        auto flashObject = XYZEngine::GameWorld::Instance()->CreateGameObject("MuzzleFlash", gameObject);

        muzzleFlashRenderer = Fx::AddSprite(flashObject, MUZZLE_FLASH_TEXTURE, FX_MUZZLE_FLASH);
        if (muzzleFlashRenderer == nullptr)
        {
            XYZEngine::GameWorld::Instance()->DestroyGameObject(flashObject);
            return;
        }
        muzzleFlashRenderer->SetVisible(false);

        muzzleFlashTransform = flashObject->GetTransform();

        muzzleFlash = Fx::AddAnimation(flashObject, MUZZLE_FLASH_TEXTURE, FX_MUZZLE_FLASH);
        muzzleFlash->SetEndBehaviour(XYZEngine::SpriteAnimationEnd::Hide);
    }

    int WeaponLayerComponent::ClampFrame(int frame, int framesCount)
    {
        return std::min(std::max(frame, 0), framesCount - 1);
    }


    int WeaponLayerComponent::GetFrameVariant(XYZEngine::MovementAnimation animation, int frame)
    {
        if (animation == XYZEngine::MovementAnimation::Reload)
        {
            return RELOAD_WEAPON_VARIANT[ClampFrame(frame, RELOAD_ANIMATION.frames)];
        }

        if (animation == XYZEngine::MovementAnimation::Swap)
        {
            return SWAP_WEAPON_VARIANT[ClampFrame(frame, SWAP_ANIMATION_FRAMES)];
        }

        return WEAPON_DEFAULT_VARIANT;
    }

    bool WeaponLayerComponent::IsWeaponHidden(XYZEngine::MovementAnimation animation, int frame)
    {
        if (animation == XYZEngine::MovementAnimation::Death)
        {
            return frame >= DEATH_WEAPON_HIDDEN_FROM_FRAME;
        }

        if (animation == XYZEngine::MovementAnimation::Swap)
        {
            return SWAP_WEAPON_HIDDEN[ClampFrame(frame, SWAP_ANIMATION_FRAMES)];
        }

        if (animation == XYZEngine::MovementAnimation::Roll)
        {
            return ROLL_WEAPON_HIDDEN[ClampFrame(frame, ROLL_ANIMATION_FRAMES)];
        }

        return false;
    }

    void WeaponLayerComponent::ShowVariant(int variant)
    {
        if (variant == currentVariant || renderer == nullptr || variants[variant] == nullptr)
        {
            return;
        }

        currentVariant = variant;
        renderer->SetTexture(*variants[variant]);
    }

    FrameOffset WeaponLayerComponent::GetFrameOffset(XYZEngine::MovementAnimation animation, int frame)
    {
        switch (animation)
        {
        case XYZEngine::MovementAnimation::Walk:
            return WALK_WEAPON_OFFSET[ClampFrame(frame, WALK_ANIMATION.frames)];
        case XYZEngine::MovementAnimation::Run:
            return RUN_WEAPON_OFFSET[ClampFrame(frame, RUN_ANIMATION.frames)];
        case XYZEngine::MovementAnimation::Shoot:
            return SHOOT_WEAPON_OFFSET[ClampFrame(frame, SHOOT_ANIMATION.frames)];
        case XYZEngine::MovementAnimation::Reload:
            return RELOAD_WEAPON_OFFSET[ClampFrame(frame, RELOAD_ANIMATION.frames)];
        case XYZEngine::MovementAnimation::Melee:
            return MELEE_WEAPON_OFFSET[ClampFrame(frame, MELEE_ANIMATION.frames)];
        case XYZEngine::MovementAnimation::Heavy:
            return HEAVY_WEAPON_OFFSET[ClampFrame(frame, HEAVY_ANIMATION_FRAMES)];
        case XYZEngine::MovementAnimation::Swap:
            return SWAP_WEAPON_OFFSET[ClampFrame(frame, SWAP_ANIMATION_FRAMES)];
        case XYZEngine::MovementAnimation::Roll:
            return ROLL_WEAPON_OFFSET[ClampFrame(frame, ROLL_ANIMATION_FRAMES)];
        case XYZEngine::MovementAnimation::Hurt:
            return HURT_WEAPON_OFFSET[ClampFrame(frame, HURT_ANIMATION.frames)];
        case XYZEngine::MovementAnimation::Death:
            return DEATH_WEAPON_OFFSET[ClampFrame(frame, DEATH_ANIMATION.frames)];
        default:
            return {0.f, 0.f};
        }
    }

    float WeaponLayerComponent::GetFrameRotation(XYZEngine::MovementAnimation animation, int frame)
    {
        switch (animation)
        {
        case XYZEngine::MovementAnimation::Melee:
            return MELEE_WEAPON_ROTATION[ClampFrame(frame, MELEE_ANIMATION.frames)];
        case XYZEngine::MovementAnimation::Heavy:
            return HEAVY_WEAPON_ROTATION[ClampFrame(frame, HEAVY_ANIMATION_FRAMES)];
        case XYZEngine::MovementAnimation::Swap:
            return SWAP_WEAPON_ROTATION[ClampFrame(frame, SWAP_ANIMATION_FRAMES)];
        default:
            return 0.f;
        }
    }
}
