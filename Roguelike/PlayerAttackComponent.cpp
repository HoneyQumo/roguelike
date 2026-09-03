#include "PlayerAttackComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <cmath>

namespace RoguelikeGame
{
    constexpr float TWO_PI = 6.2831853f;

    PlayerAttackComponent::PlayerAttackComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
    }

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
        if (meleeWeapon == nullptr)
        {
            meleeWeapon = gameObject->GetComponent<XYZEngine::MeleeWeaponComponent>();
        }
        if (health == nullptr)
        {
            health = gameObject->GetComponent<XYZEngine::HealthComponent>();
        }
        if (dodgeRoll == nullptr)
        {
            dodgeRoll = gameObject->GetComponent<XYZEngine::DodgeRollComponent>();
        }
        if (loadout == nullptr)
        {
            loadout = gameObject->GetComponent<PlayerLoadoutComponent>();
        }
        if (hitFlash == nullptr)
        {
            hitFlash = gameObject->GetComponent<HitFlashComponent>();
        }

        if (input == nullptr || weapon == nullptr || meleeWeapon == nullptr || loadout == nullptr)
        {
            LOG_ERROR("Player attack needs input, weapon, melee weapon and loadout components");
            gameObject->RemoveComponent(this);
            return;
        }

        if (health != nullptr && !health->IsAlive())
        {
            return;
        }

        UpdateChargeGlow(deltaTime);

        if (dodgeRoll != nullptr && dodgeRoll->IsRolling())
        {
            return;
        }

        if (loadout->IsSwapping())
        {
            return;
        }

        if (loadout->IsMeleeEquipped())
        {
            UpdateMelee(deltaTime);
            return;
        }

        UpdateRanged();
    }

    void PlayerAttackComponent::Render()
    {
    }

    void PlayerAttackComponent::UpdateMelee(float deltaTime)
    {
        if (input->IsHeavyAttackPressed())
        {
            meleeWeapon->TryStartHeavyAttack();
        }
        else
        {
            meleeWeapon->ReleaseHeavyAttack();

            if (input->IsAttackPressed())
            {
                meleeWeapon->TryQuickAttack();
            }
        }
    }

    void PlayerAttackComponent::UpdateRanged()
    {
        if (input->IsReloadPressed())
        {
            weapon->TryReload();
        }

        if (input->IsAttackPressed())
        {
            weapon->TryShootAt(input->GetMouseWorldPosition());
        }
    }

    void PlayerAttackComponent::UpdateChargeGlow(float deltaTime)
    {
        if (hitFlash == nullptr)
        {
            return;
        }

        if (!meleeWeapon->IsCharging() || !meleeWeapon->IsCharged())
        {
            glowTimer = 0.f;
            hitFlash->SetGlow(0.f);
            return;
        }

        glowTimer += deltaTime;
        float phase = TWO_PI * glowTimer / HEAVY_CHARGED_GLOW_PERIOD;
        hitFlash->SetGlow(HEAVY_CHARGED_GLOW * (0.5f + 0.5f * std::sin(phase)));
    }
}
