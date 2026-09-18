#include "QuickBeltComponent.h"
#include "GameSettings.h"
#include "InventoryAccess.h"
#include <InputSystem.h>
#include <UiManager.h>

namespace RoguelikeGame
{
    QuickBeltComponent::QuickBeltComponent(XYZEngine::GameObject* gameObject)
        : Component(gameObject), hooks(QUICK_BELT_HOOKS)
    {
    }

    void QuickBeltComponent::Start()
    {
        if (inventory == nullptr)
        {
            return;
        }

        // Подписка на добавление, а не на подбор: в сумку попадает только то,
        // что не применилось на месте, и патроны на крючок не просочатся.
        inventory->SubscribeAdded([this](const ItemDefinition& item, int)
        {
            HangOnFreeHook(item);
        });
    }

    void QuickBeltComponent::Update(float deltaTime)
    {
        sinceUse += deltaTime;

        ReadKeys();
    }

    void QuickBeltComponent::ReadKeys()
    {
        // Открытая сумка забирает ввод: там те же цифры вешают предмет на крючок.
        if (XYZEngine::UiManager::Instance()->IsInputCaptured())
        {
            return;
        }

        auto input = XYZEngine::InputSystem::Instance();

        for (int hook = 0; hook < GetHooksCount(); hook++)
        {
            auto action = static_cast<XYZEngine::InputAction>(
                static_cast<int>(XYZEngine::InputAction::QuickSlot1) + hook);

            if (input->WasActionPressed(action))
            {
                UseHook(hook);
                return;
            }
        }
    }

    bool QuickBeltComponent::UseHook(int hook)
    {
        // Проверка живёт здесь, а не только у чтения клавиш: применить с пояса
        // нельзя и тогда, когда просят напрямую - в катсцене игрок заморожен.
        if (!IsValidHook(hook) || hooks[hook].empty() || !CanUseInventory(inventory))
        {
            return false;
        }

        // Без задержки одно нажатие успевает выпить две аптечки за соседние кадры.
        if (sinceUse < QUICK_BELT_COOLDOWN)
        {
            return false;
        }

        int slot = inventory->FindSlot(hooks[hook]);
        if (slot < 0)
        {
            return false;
        }

        sinceUse = 0.f;

        return inventory->Use(slot);
    }

    bool QuickBeltComponent::Bind(int hook, const ItemDefinition& item)
    {
        if (!IsValidHook(hook) || !CanHang(item))
        {
            return false;
        }

        // Один предмет не висит на двух крючках: иначе счётчики показывают одно и то же.
        for (std::string& taken : hooks)
        {
            if (taken == item.id)
            {
                taken.clear();
            }
        }

        hooks[hook] = item.id;

        return true;
    }

    const std::string& QuickBeltComponent::GetBinding(int hook) const
    {
        static const std::string EMPTY;

        return IsValidHook(hook) ? hooks[hook] : EMPTY;
    }

    int QuickBeltComponent::GetCountOn(int hook) const
    {
        if (!IsValidHook(hook) || hooks[hook].empty() || inventory == nullptr)
        {
            return 0;
        }

        return inventory->CountOf(hooks[hook]);
    }

    int QuickBeltComponent::GetHooksCount() const
    {
        return static_cast<int>(hooks.size());
    }

    void QuickBeltComponent::SetInventory(InventoryComponent* newInventory)
    {
        inventory = newInventory;
    }

    void QuickBeltComponent::SetEffects(ItemEffectComponent* newEffects)
    {
        effects = newEffects;
    }

    bool QuickBeltComponent::IsValidHook(int hook) const
    {
        return hook >= 0 && hook < GetHooksCount();
    }

    // Ключ на крючке был бы мёртвой кнопкой: применять его некому.
    bool QuickBeltComponent::CanHang(const ItemDefinition& item) const
    {
        return item.type == ItemType::Consumable && effects != nullptr && effects->HasHandler(item.effect.kind);
    }

    void QuickBeltComponent::HangOnFreeHook(const ItemDefinition& item)
    {
        if (!CanHang(item))
        {
            return;
        }

        for (const std::string& taken : hooks)
        {
            if (taken == item.id)
            {
                return;
            }
        }

        for (std::string& hook : hooks)
        {
            if (hook.empty())
            {
                hook = item.id;
                return;
            }
        }
    }
}
