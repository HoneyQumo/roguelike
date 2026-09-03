#include "PlayerRollComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>

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

        if (input == nullptr || dodgeRoll == nullptr)
        {
            LOG_ERROR("Player roll needs input and dodge roll components");
            gameObject->DestroyComponent(this);
        }
    }

    void PlayerRollComponent::Update(float deltaTime)
    {
        if (!input->WasRollJustPressed() || !CanRoll())
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
