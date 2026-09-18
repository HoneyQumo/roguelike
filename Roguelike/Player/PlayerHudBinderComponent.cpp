#include "PlayerHudBinderComponent.h"
#include "GameResources.h"
#include <ResourceSystem.h>
#include <InputSystem.h>
#include "Item.h"
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

    // Отказ показывается там, где нажали: HUD лежит под затемнением открытой сумки.
    void PlayerHudBinderComponent::ShowRefusal(const std::string& text)
    {
        if (inventoryScreen != nullptr && inventoryScreen->IsOpen())
        {
            inventoryScreen->ShowNotice(text);
            return;
        }

        if (screen != nullptr)
        {
            screen->ShowNotice(text);
        }
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
            PushWeaponSlots();
        }

        if (health != nullptr || stamina != nullptr)
        {
            screen->SetVitals(ReadVitalsState());
        }
    }

    void PlayerHudBinderComponent::Render()
    {
    }

    /**
    *	Ряд слотов собирает биндер: он один знает и раскладку, и каталог предметов,
    *	и привязки клавиш. Экран рисует то, что дали, и ни о чём из этого не знает.
    */
    void PlayerHudBinderComponent::PushWeaponSlots()
    {
        const LoadoutState& state = loadout->GetState();
        std::vector<WeaponSlotHudState> slots;

        for (int slot = 0; slot < state.slotsCount; slot++)
        {
            WeaponSlotHudState shown;
            shown.hasWeapon = !state.IsEmpty(slot);
            shown.isCurrent = slot == state.currentSlot;

            auto action = static_cast<XYZEngine::InputAction>(
                static_cast<int>(XYZEngine::InputAction::WeaponSlot1) + slot);
            shown.key = DigitOfKey(XYZEngine::InputSystem::Instance()->GetBinding(action).key);

            if (shown.hasWeapon)
            {
                const ItemDefinition* item = FindWeaponItem(GameResources::GetItems(), GetWeapon(state.slots[slot].id).id);
                if (item != nullptr)
                {
                    shown.icon = XYZEngine::ResourceSystem::Instance()->GetTextureShared(ItemTextureName(item->id));
                }
            }

            slots.push_back(shown);
        }

        screen->SetWeaponSlots(slots);
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

            interaction->SubscribeRefused([this](const std::string& reason)
            {
                if (screen != nullptr && !reason.empty())
                {
                    screen->ShowNotice(reason);
                }
            });
        }

        inventory = target->GetComponent<InventoryComponent>();
        if (inventory != nullptr && inventoryScreen != nullptr)
        {
            inventoryScreen->SetInventory(inventory);

            inventoryScreen->SetEquipHandler([this](int bagSlot, int targetSlot)
            {
                if (inventory == nullptr || loadout == nullptr)
                {
                    return false;
                }

                EquipResult result = loadout->EquipFromBag(*inventory, bagSlot, targetSlot, GameResources::GetItems());
                if (!result.isDone)
                {
                    ShowRefusal(EquipRefuseText(result.refusal));
                }

                return result.isDone;
            });
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
            effects->SubscribeRefused([this](const ItemDefinition&, ItemRefuseReason reason)
            {
                ShowRefusal(ItemRefuseText(reason));
            });
        }
    }

    VitalsHudState PlayerHudBinderComponent::ReadVitalsState() const
    {
        VitalsHudState state;

        if (health != nullptr)
        {
            state.healthPart = health->GetHealthPercent();
            state.armorPart = health->GetArmorPercent();
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
        if (!loadout->HasWeapon())
        {
            state.weaponName = EMPTY_SLOT_NAME;
            return state;
        }

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
