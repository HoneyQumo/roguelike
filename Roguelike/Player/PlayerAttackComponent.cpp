#include "PlayerAttackComponent.h"
#include <MathUtils.h>
#include "GameSettings.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <cmath>
#include <UiManager.h>

namespace RoguelikeGame
{

    PlayerAttackComponent::PlayerAttackComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
    }

    void PlayerAttackComponent::Start()
    {
        input = gameObject->GetComponent<XYZEngine::InputComponent>();
        weapon = gameObject->GetComponent<WeaponComponent>();
        meleeWeapon = gameObject->GetComponent<MeleeWeaponComponent>();
        health = gameObject->GetComponent<HealthComponent>();
        dodgeRoll = gameObject->GetComponent<DodgeRollComponent>();
        loadout = gameObject->GetComponent<PlayerLoadoutComponent>();
        hitFlash = gameObject->GetComponent<HitFlashComponent>();
        stamina = gameObject->GetComponent<StaminaComponent>();

        if (input == nullptr || weapon == nullptr || meleeWeapon == nullptr || loadout == nullptr)
        {
            LOG_ERROR("Player attack needs input, weapon, melee weapon and loadout components");
            gameObject->DestroyComponent(this);
        }
    }

    void PlayerAttackComponent::Update(float deltaTime)
    {
        if (health != nullptr && !health->IsAlive())
        {
            return;
        }

        UpdateChargeGlow(deltaTime);

        if (XYZEngine::UiManager::Instance()->IsPointerCaptured())
        {
            // Отпускание зажатого удара живёт в UpdateMelee, до которого мы отсюда
            // не дойдём, а замах держит движение выключенным - игрок встанет
            // вкопанным. Так же поступают перекат и смена оружия.
            meleeWeapon->CancelAttack();
            return;
        }

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
        if (input->IsActionHeld(XYZEngine::InputAction::HeavyAttack))
        {
            if (stamina == nullptr || stamina->HasStamina(PLAYER_HEAVY_ATTACK_STAMINA))
            {
                if (meleeWeapon->TryStartHeavyAttack() && stamina != nullptr)
                {
                    stamina->TrySpend(PLAYER_HEAVY_ATTACK_STAMINA);
                }
            }
        }
        else
        {
            meleeWeapon->ReleaseHeavyAttack();

            if (input->IsActionHeld(XYZEngine::InputAction::Attack))
            {
                meleeWeapon->TryQuickAttack();
            }
        }
    }

    void PlayerAttackComponent::UpdateRanged()
    {
        if (input->IsActionHeld(XYZEngine::InputAction::Reload))
        {
            weapon->TryReload();
        }

        if (input->IsActionHeld(XYZEngine::InputAction::Attack))
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
        float phase = XYZEngine::TWO_PI * glowTimer / HEAVY_CHARGED_GLOW_PERIOD;
        hitFlash->SetGlow(HEAVY_CHARGED_GLOW * (0.5f + 0.5f * std::sin(phase)));
    }
}
