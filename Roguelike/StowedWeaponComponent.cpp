#include "StowedWeaponComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <ResourceSystem.h>
#include <algorithm>

namespace RoguelikeGame
{
    StowedWeaponComponent::StowedWeaponComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
    }

    void StowedWeaponComponent::Update(float deltaTime)
    {
        if (renderer == nullptr)
        {
            renderer = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();
            if (renderer == nullptr)
            {
                return;
            }
        }

        if (ownerAnimation == nullptr)
        {
            return;
        }

        int frame = std::min(std::max(ownerAnimation->GetCurrentFrame(), 0), SWAP_ANIMATION_FRAMES - 1);
        bool isShown = texture != nullptr
            && ownerAnimation->GetCurrentAnimation() == XYZEngine::MovementAnimation::Swap
            && SWAP_WEAPON_HIDDEN[frame];

        renderer->SetVisible(isShown);
        if (!isShown)
        {
            return;
        }

        transform->SetLocalPosition(ToWorldOffset(SWAP_STOW_OFFSET[frame].x, SWAP_STOW_OFFSET[frame].y));
        transform->SetLocalRotation(ToWorldAngle(SWAP_STOW_ROTATION[frame]));
    }

    void StowedWeaponComponent::Render()
    {
    }

    void StowedWeaponComponent::SetOwnerAnimation(XYZEngine::SpriteMovementAnimationComponent* newOwnerAnimation)
    {
        ownerAnimation = newOwnerAnimation;
    }

    void StowedWeaponComponent::SetWeaponId(WeaponId newWeaponId)
    {
        texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(WEAPONS_TEXTURE,
                                                                                    WeaponFrameIndex(newWeaponId, WEAPON_STOWED_VARIANT));

        if (renderer == nullptr)
        {
            renderer = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();
        }

        if (renderer != nullptr && texture != nullptr)
        {
            renderer->SetTexture(*texture);
        }
    }
}
