#include "PlayerAttackComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    PlayerAttackComponent::PlayerAttackComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void PlayerAttackComponent::Update(float deltaTime)
    {
        if (input == nullptr)
        {
            input = gameObject->GetComponent<XYZEngine::InputComponent>();
        }
        if (weapon == nullptr)
        {
            weapon = gameObject->GetComponent<XYZEngine::WeaponComponent>();
        }
        if (health == nullptr)
        {
            health = gameObject->GetComponent<XYZEngine::HealthComponent>();
        }
        if (aim == nullptr)
        {
            aim = gameObject->GetComponent<XYZEngine::AimRotationComponent>();
        }

        if (input == nullptr || weapon == nullptr || aim == nullptr)
        {
            LOG_ERROR("Player attack needs input, weapon and aim components");
            gameObject->RemoveComponent(this);
            return;
        }

        if (health != nullptr && !health->IsAlive())
        {
            return;
        }

        if (input->IsAttackPressed())
        {
            weapon->TryShoot(aim->GetAimDirection());
        }
    }

    void PlayerAttackComponent::Render()
    {
    }
}
