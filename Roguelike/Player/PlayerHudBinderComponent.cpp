#include "PlayerHudBinderComponent.h"
#include "GameResources.h"
#include <ResourceSystem.h>
#include <InputSystem.h>
#include "Item.h"
#include "GameSettings.h"
#include "WeaponCatalog.h"
#include <GameWorld.h>
#include <RenderSystem.h>

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

    void PlayerHudBinderComponent::SetThreatMarks(ThreatMarkComponent* newThreatMarks)
    {
        threatMarks = newThreatMarks;
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
            PushWeaponSlots();
        }

        if (belt != nullptr)
        {
            PushBeltSlots();
        }

        if (threats != nullptr && threatMarks != nullptr && threats->GetGameObject() != nullptr)
        {
            threatMarks->SetMarks(BuildThreatMarks(threats->GetSources(),
                threats->GetGameObject()->GetTransform()->GetWorldPosition(),
                XYZEngine::RenderSystem::Instance()->GetViewArea()));
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
    *	Патроны ячейки. У слота в руках берём живые - магазин лежит в самом оружии.
    *	У отложенных берём тот, что помнит раскладка: туда он пишется при смене слота
    *	и при обмене. Запас общий по виду патронов и лежит в подсумке.
    */
    void PlayerHudBinderComponent::FillAmmo(SlotHudState& shown, WeaponId id, bool isCurrent, int remembered) const
    {
        const WeaponDefinition& definition = GetWeapon(id);
        if (definition.magazineSize <= 0)
        {
            return;
        }

        shown.hasCount = true;
        shown.count = isCurrent && weapon != nullptr ? weapon->GetAmmoInMagazine() : remembered;
        shown.isReloading = isCurrent && weapon != nullptr && weapon->IsReloading();
        shown.isLow = shown.count <= static_cast<int>(definition.magazineSize * AMMO_HUD_LOW_PART);
        shown.reserve = pouch != nullptr ? pouch->GetAmmo(AmmoKindKey(definition.ammo)) : NO_RESERVE;
    }

    /**
    *	Ряд слотов собирает биндер: он один знает и раскладку, и каталог предметов,
    *	и привязки клавиш. Экран рисует то, что дали, и ни о чём из этого не знает.
    */
    void PlayerHudBinderComponent::PushWeaponSlots()
    {
        const LoadoutState& state = loadout->GetState();
        std::vector<SlotHudState> slots;

        for (int slot = 0; slot < state.slotsCount; slot++)
        {
            SlotHudState shown;
            shown.isFilled = !state.IsEmpty(slot);
            shown.isCurrent = slot == state.currentSlot;

            auto action = static_cast<XYZEngine::InputAction>(
                static_cast<int>(XYZEngine::InputAction::WeaponSlot1) + slot);
            shown.key = DigitOfKey(XYZEngine::InputSystem::Instance()->GetBinding(action).key);

            if (shown.isFilled)
            {
                WeaponId id = state.slots[slot].id;
                const ItemDefinition* item = FindWeaponItem(GameResources::GetItems(), GetWeapon(id).id);
                if (item != nullptr)
                {
                    shown.icon = XYZEngine::ResourceSystem::Instance()->GetTextureShared(ItemTextureName(item->id));
                }

                FillAmmo(shown, id, slot == state.currentSlot, state.slots[slot].magazine);
            }

            slots.push_back(shown);
        }

        screen->SetWeaponSlots(slots);
    }

    /**
    *	Пояс на экране: иконка и количество берутся у сумки по id, который помнит крючок.
    *	Своего числа у пояса нет, поэтому показанное не может разойтись с содержимым.
    */
    void PlayerHudBinderComponent::PushBeltSlots()
    {
        std::vector<SlotHudState> slots;

        for (int hook = 0; hook < belt->GetHooksCount(); hook++)
        {
            SlotHudState shown;

            auto action = static_cast<XYZEngine::InputAction>(
                static_cast<int>(XYZEngine::InputAction::QuickSlot1) + hook);
            shown.key = DigitOfKey(XYZEngine::InputSystem::Instance()->GetBinding(action).key);

            const std::string& carried = belt->GetBinding(hook);
            if (!carried.empty())
            {
                const ItemDefinition* item = GameResources::GetItems().Find(carried);

                shown.isFilled = true;
                shown.hasCount = true;
                shown.count = belt->GetCountOn(hook);
                shown.isLow = shown.count == 0;

                if (item != nullptr)
                {
                    shown.icon = XYZEngine::ResourceSystem::Instance()->GetTextureShared(ItemTextureName(item->id));
                }
            }

            slots.push_back(shown);
        }

        screen->SetBeltSlots(slots);
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

        pouch = target->GetComponent<AmmoPouchComponent>();
        belt = target->GetComponent<QuickBeltComponent>();
        drop = target->GetComponent<ItemDropComponent>();
        threats = target->GetComponent<ThreatWatchComponent>();
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

            inventoryScreen->SetBeltHandler([this](int bagSlot, int hook)
            {
                if (inventory == nullptr || belt == nullptr)
                {
                    return false;
                }

                const InventorySlot& carried = inventory->GetSlot(bagSlot);
                if (carried.IsEmpty() || !belt->Bind(hook, carried.item))
                {
                    ShowRefusal(BELT_REFUSED_NOTICE);
                    return false;
                }

                return true;
            });

            auto effects = target->GetComponent<ItemEffectComponent>();
            inventoryScreen->SetUsableRule([effects](const ItemDefinition& item)
            {
                return effects != nullptr && effects->HasHandler(item.effect.kind);
            });

            inventoryScreen->SetDropHandler([this](int bagSlot)
            {
                return drop != nullptr && drop->Drop(bagSlot);
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

}
