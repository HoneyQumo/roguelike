#include "PlayerRollComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include "GameSettings.h"

namespace RoguelikeGame
{
    PlayerRollComponent::PlayerRollComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
    }

    void PlayerRollComponent::Start()
    {
        input = gameObject->GetComponent<XYZEngine::InputComponent>();
        dodgeRoll = gameObject->GetComponent<DodgeRollComponent>();
        meleeWeapon = gameObject->GetComponent<MeleeWeaponComponent>();
        health = gameObject->GetComponent<HealthComponent>();
        loadout = gameObject->GetComponent<PlayerLoadoutComponent>();
        stamina = gameObject->GetComponent<StaminaComponent>();

        if (input == nullptr || dodgeRoll == nullptr)
        {
            LOG_ERROR("Player roll needs input and dodge roll components");
            gameObject->DestroyComponent(this);
        }
    }

    void PlayerRollComponent::Update(float deltaTime)
    {
        if (!input->WasActionPressed(XYZEngine::InputAction::Roll) || !CanRoll())
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

        if (stamina != nullptr && !stamina->TrySpend(PLAYER_ROLL_STAMINA))
        {
            return;
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
