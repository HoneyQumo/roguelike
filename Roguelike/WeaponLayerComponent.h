#pragma once

#include <Component.h>
#include <TransformComponent.h>
#include <SpriteRendererComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include "SpriteAtlas.h"

namespace RoguelikeGame
{
    class WeaponLayerComponent : public XYZEngine::Component
    {
    public:
        WeaponLayerComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetOwnerAnimation(XYZEngine::SpriteMovementAnimationComponent* newOwnerAnimation);
        void SetRecoil(float newRecoil);

    private:
        XYZEngine::TransformComponent* transform;
        XYZEngine::SpriteRendererComponent* renderer = nullptr;
        XYZEngine::SpriteMovementAnimationComponent* ownerAnimation = nullptr;

        float recoil = 1.f;

        static FrameOffset GetFrameOffset(XYZEngine::MovementAnimation animation, int frame);
    };
}
