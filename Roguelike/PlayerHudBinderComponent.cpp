#include "PlayerHudBinderComponent.h"
#include "GameSettings.h"
#include "WeaponCatalog.h"
#include <GameWorld.h>

namespace RoguelikeGame
{
    PlayerHudBinderComponent::PlayerHudBinderComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void PlayerHudBinderComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
        weapon = nullptr;
        loadout = nullptr;
        health = nullptr;
        stamina = nullptr;
        inventory = nullptr;
        interaction = nullptr;
    }

    void PlayerHudBinderComponent::SetScreen(HudScreen* newScreen)
    {
        screen = newScreen;
    }

    void PlayerHudBinderComponent::SetInventoryScreen(InventoryScreen* newInventoryScreen)
    {
        inventoryScreen = newInventoryScreen;
    }

    void PlayerHudBinderComponent::Update(float deltaTime)
    {
        if (loadout == nullptr || health == nullptr)
        {
            FindTarget();
        }

        if (screen == nullptr)
        {
            return;
        }

        if (loadout != nullptr)
        {
            screen->SetAmmo(ReadAmmoState());
        }

        if (health != nullptr || stamina != nullptr)
        {
            screen->SetVitals(ReadVitalsState());
        }
    }

    void PlayerHudBinderComponent::Render()
    {
    }

    void PlayerHudBinderComponent::FindTarget()
    {
        if (targetName.empty())
        {
            return;
        }

        XYZEngine::GameObject* target = XYZEngine::GameWorld::Instance()->FindGameObject(targetName);
        if (target == nullptr)
        {
            return;
        }

        weapon = target->GetComponent<WeaponComponent>();
        loadout = target->GetComponent<PlayerLoadoutComponent>();
        health = target->GetComponent<HealthComponent>();
        stamina = target->GetComponent<StaminaComponent>();

        interaction = target->GetComponent<InteractionComponent>();
        if (interaction != nullptr)
        {
            interaction->SubscribePromptChanged([this](const std::string& prompt)
            {
                if (screen != nullptr)
                {
                    screen->SetPrompt(prompt);
                }
            });
        }

        inventory = target->GetComponent<InventoryComponent>();
        if (inventory != nullptr && inventoryScreen != nullptr)
        {
            inventoryScreen->SetInventory(inventory);
        }

        if (inventory != nullptr)
        {
            inventory->SubscribeRejected([this](const ItemDefinition&)
            {
                if (screen != nullptr)
                {
                    screen->ShowNotice(INVENTORY_FULL_NOTICE);
                }
            });
        }

        auto effects = target->GetComponent<ItemEffectComponent>();
        if (effects != nullptr)
        {
            effects->SubscribeRefused([this](const ItemDefinition&)
            {
                if (screen != nullptr)
                {
                    screen->ShowNotice(ITEM_REFUSED_NOTICE);
                }
            });
        }
    }

    VitalsHudState PlayerHudBinderComponent::ReadVitalsState() const
    {
        VitalsHudState state;

        if (health != nullptr)
        {
            state.healthPart = health->GetHealthPercent();
        }

        if (stamina != nullptr)
        {
            state.staminaPart = stamina->GetStaminaPercent();
            state.isExhausted = stamina->IsExhausted();
        }

        return state;
    }

    AmmoHudState PlayerHudBinderComponent::ReadAmmoState() const
    {
        AmmoHudState state;
        state.weaponName = GetWeapon(loadout->GetCurrentWeapon()).name;

        if (weapon == nullptr || !weapon->HasMagazine())
        {
            return state;
        }

        state.hasMagazine = true;
        state.inMagazine = weapon->GetAmmoInMagazine();
        state.reserve = weapon->GetReserveAmmo();
        state.isReloading = weapon->IsReloading();
        state.isLow = weapon->GetAmmoInMagazine() <= static_cast<int>(weapon->GetMagazineSize() * AMMO_HUD_LOW_PART);

        return state;
    }
}
