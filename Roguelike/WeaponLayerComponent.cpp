#include "WeaponLayerComponent.h"
#include <GameObject.h>
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
        case XYZEngine::MovementAnimation::Hurt:
            return HURT_WEAPON_OFFSET[std::min(std::max(frame, 0), HURT_ANIMATION.frames - 1)];
        case XYZEngine::MovementAnimation::Death:
            return DEATH_WEAPON_OFFSET[std::min(std::max(frame, 0), DEATH_ANIMATION.frames - 1)];
        default:
            return {0, 0};
        }
    }
}
