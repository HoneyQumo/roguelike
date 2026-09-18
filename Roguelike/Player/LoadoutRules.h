#pragma once

#include <algorithm>
#include "GameSettings.h"
#include "WeaponCatalog.h"

namespace RoguelikeGame
{
    /**
    *	Раскладка оружия отдельно от компонента.
    *
    *	У компонента за спиной анимация, звук, спрайт ствола и ресурсы, поэтому в
    *	тестовый проект он не попадает. Сами правила слотов знать про это не обязаны
    *	и живут здесь - без GameObject и без SFML.
    */
    struct LoadoutSlot
    {
        WeaponId id = WeaponId::Knife;
        int magazine = 0;
        bool hasWeapon = false;
    };

    struct EquipOutcome
    {
        bool isChanged = false;
        int slot = NO_WEAPON_SLOT;
    };

    struct LoadoutState
    {
        LoadoutSlot slots[PLAYER_WEAPON_SLOTS] = {};
        int slotsCount = 0;
        int currentSlot = 0;

        constexpr bool IsEmpty(int slot) const
        {
            return slot < 0 || slot >= slotsCount || !slots[slot].hasWeapon;
        }

        constexpr WeaponId Current() const
        {
            return slots[currentSlot].id;
        }

        // У пустой раскладки слота нет: clamp с lo > hi - это UB, а не защита.
        constexpr int ResolveSlot(WeaponId id) const
        {
            return slotsCount <= 0 ? NO_WEAPON_SLOT : std::clamp(PreferredWeaponSlot(id), 0, slotsCount - 1);
        }

        constexpr bool CanTake(WeaponId id) const
        {
            int slot = ResolveSlot(id);

            return slot != NO_WEAPON_SLOT && IsEmpty(slot);
        }

        constexpr bool CanSelect(int slot) const
        {
            return slot != currentSlot && !IsEmpty(slot);
        }

        // Стартовый слот может оказаться пустым: тогда берём первый вооружённый.
        constexpr int FirstArmed(int wanted) const
        {
            if (slotsCount <= 0)
            {
                return NO_WEAPON_SLOT;
            }

            if (!IsEmpty(wanted))
            {
                return std::clamp(wanted, 0, slotsCount - 1);
            }

            for (int slot = 0; slot < slotsCount; slot++)
            {
                if (!IsEmpty(slot))
                {
                    return slot;
                }
            }

            return NO_WEAPON_SLOT;
        }

        constexpr void Fill(const StartingSlot* newSlots, int newSlotsCount)
        {
            slotsCount = std::min(newSlotsCount, PLAYER_WEAPON_SLOTS);

            for (int slot = 0; slot < slotsCount; slot++)
            {
                slots[slot].id = newSlots[slot].id;
                slots[slot].hasWeapon = newSlots[slot].hasWeapon;
                slots[slot].magazine = newSlots[slot].hasWeapon ? GetWeapon(newSlots[slot].id).magazineSize : 0;
            }
        }

        // Занятый слот сейчас затирается вместе с тем, что в нём лежало - см. F-GPL-66.
        constexpr EquipOutcome Equip(WeaponId id)
        {
            int slot = ResolveSlot(id);
            if (slot == NO_WEAPON_SLOT || (slots[slot].hasWeapon && slots[slot].id == id))
            {
                return {};
            }

            slots[slot].id = id;
            slots[slot].hasWeapon = true;
            slots[slot].magazine = GetWeapon(id).magazineSize;

            return {true, slot};
        }
    };
}
