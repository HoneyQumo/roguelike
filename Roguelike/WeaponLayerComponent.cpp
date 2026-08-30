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

        bool isHidden = animation == XYZEngine::MovementAnimation::Death && frame >= DEATH_WEAPON_HIDDEN_FROM_FRAME;
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
        float forward = animation == XYZEngine::MovementAnimation::Shoot ? offset.x * recoil : (float)offset.x;
        transform->SetLocalPosition(ToWorldOffset(forward, (float)offset.y));
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

        currentVariant = WEAPON_DEFAULT_VARIANT;
    }

    // Ствол меняет хват только на перезарядке, во всех прочих кадрах он в обычном варианте.
    int WeaponLayerComponent::GetFrameVariant(XYZEngine::MovementAnimation animation, int frame)
    {
        if (animation != XYZEngine::MovementAnimation::Reload)
        {
            return WEAPON_DEFAULT_VARIANT;
        }

        return RELOAD_WEAPON_VARIANT[std::min(std::max(frame, 0), RELOAD_ANIMATION.frames - 1)];
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
            return WALK_WEAPON_OFFSET[std::min(std::max(frame, 0), WALK_ANIMATION.frames - 1)];
        case XYZEngine::MovementAnimation::Run:
            return RUN_WEAPON_OFFSET[std::min(std::max(frame, 0), RUN_ANIMATION.frames - 1)];
        case XYZEngine::MovementAnimation::Shoot:
            return SHOOT_WEAPON_OFFSET[std::min(std::max(frame, 0), SHOOT_ANIMATION.frames - 1)];
        case XYZEngine::MovementAnimation::Reload:
            return RELOAD_WEAPON_OFFSET[std::min(std::max(frame, 0), RELOAD_ANIMATION.frames - 1)];
        case XYZEngine::MovementAnimation::Hurt:
            return HURT_WEAPON_OFFSET[std::min(std::max(frame, 0), HURT_ANIMATION.frames - 1)];
        case XYZEngine::MovementAnimation::Death:
            return DEATH_WEAPON_OFFSET[std::min(std::max(frame, 0), DEATH_ANIMATION.frames - 1)];
        default:
            return {0, 0};
        }
    }
}
