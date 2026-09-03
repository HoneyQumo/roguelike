#include "PlayerRollComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    PlayerRollComponent::PlayerRollComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
    }

    void PlayerRollComponent::Update(float deltaTime)
    {
        if (input == nullptr)
        {
            input = gameObject->GetComponent<XYZEngine::InputComponent>();
        }
        if (dodgeRoll == nullptr)
        {
            dodgeRoll = gameObject->GetComponent<XYZEngine::DodgeRollComponent>();
        }
        if (meleeWeapon == nullptr)
        {
            meleeWeapon = gameObject->GetComponent<XYZEngine::MeleeWeaponComponent>();
        }
        if (health == nullptr)
        {
            health = gameObject->GetComponent<XYZEngine::HealthComponent>();
        }
        if (loadout == nullptr)
        {
            loadout = gameObject->GetComponent<PlayerLoadoutComponent>();
        }

        if (input == nullptr || dodgeRoll == nullptr)
        {
            LOG_ERROR("Player roll needs input and dodge roll components");
            gameObject->RemoveComponent(this);
            return;
        }

        bool isRollPressed = input->IsRollPressed();
        bool isRollJustPressed = isRollPressed && !wasRollPressed;
        wasRollPressed = isRollPressed;

        if (!isRollJustPressed || !CanRoll())
        {
            return;
        }

        if (loadout != nullptr)
        {
            loadout->CancelReload();
        }

        if (meleeWeapon != nullptr)
        {
            meleeWeapon->CancelAttack();
        }

        dodgeRoll->TryRoll({input->GetHorizontalAxis(), input->GetVerticalAxis()});
    }

    void PlayerRollComponent::Render()
    {
    }

    bool PlayerRollComponent::CanRoll() const
    {
        if (health != nullptr && !health->IsAlive())
        {
            return false;
        }

        if (loadout != nullptr && loadout->IsSwapping())
        {
            return false;
        }

        return dodgeRoll->IsReady();
    }
}
