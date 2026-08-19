#pragma once

#include <Component.h>
#include <TransformComponent.h>
#include <InputComponent.h>
#include <WeaponComponent.h>
#include <HealthComponent.h>
#include <SpriteRendererComponent.h>
#include <Vector.h>

namespace RoguelikeGame
{
    class PlayerAttackComponent : public XYZEngine::Component
    {
    public:
        PlayerAttackComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetWeapon(XYZEngine::TransformComponent* weaponTransform, XYZEngine::SpriteRendererComponent* weaponRenderer);

    private:
        XYZEngine::TransformComponent* transform;
        XYZEngine::InputComponent* input = nullptr;
        XYZEngine::WeaponComponent* weapon = nullptr;
        XYZEngine::HealthComponent* health = nullptr;

        XYZEngine::TransformComponent* weaponTransform = nullptr;
        XYZEngine::SpriteRendererComponent* weaponRenderer = nullptr;

        void AimWeapon(const XYZEngine::Vector2Df& direction);
    };
}
