#include "WeaponLayerComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <ResourceSystem.h>
#include <algorithm>

namespace RoguelikeGame
{
    WeaponLayerComponent::WeaponLayerComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
    }

    void WeaponLayerComponent::Update(float deltaTime)
    {
        if (renderer == nullptr)
        {
            renderer = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();
        }

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
        float forward = animation == XYZEngine::MovementAnimation::Shoot ? offset.x * recoil : static_cast<float>(offset.x);
        transform->SetLocalPosition(ToWorldOffset(forward, static_cast<float>(offset.y)));
        transform->SetLocalRotation(ToWorldAngle(GetFrameRotation(animation, frame)));
    }

    void WeaponLayerComponent::Render()
    {
    }

    void WeaponLayerComponent::SetOwnerAnimation(XYZEngine::SpriteMovementAnimationComponent* newOwnerAnimation)
    {
        ownerAnimation = newOwnerAnimation;
    }

    void WeaponLayerComponent::SetRecoil(float newRecoil)
    {
        recoil = newRecoil;
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
        case XYZEngine::MovementAnimation::Hurt:
            return HURT_WEAPON_OFFSET[ClampFrame(frame, HURT_ANIMATION.frames)];
        case XYZEngine::MovementAnimation::Death:
            return DEATH_WEAPON_OFFSET[ClampFrame(frame, DEATH_ANIMATION.frames)];
        default:
            return {0, 0};
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
