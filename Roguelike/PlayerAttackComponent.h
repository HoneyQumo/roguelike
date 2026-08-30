#pragma once

#include <Component.h>
#include <InputComponent.h>
#include <WeaponComponent.h>
#include <HealthComponent.h>
#include <AimRotationComponent.h>

namespace RoguelikeGame
{
    // Отвечает за аттаку. За направление отвечает AimRotationComponent.
    class PlayerAttackComponent : public XYZEngine::Component
    {
    public:
        PlayerAttackComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

    private:
        XYZEngine::InputComponent* input = nullptr;
        XYZEngine::WeaponComponent* weapon = nullptr;
        XYZEngine::HealthComponent* health = nullptr;
        XYZEngine::AimRotationComponent* aim = nullptr;
    };
}
