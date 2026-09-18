#include "EquipExchange.h"

namespace RoguelikeGame
{
    namespace
    {
        EquipResult Refuse(EquipRefusal refusal)
        {
            EquipResult result;
            result.refusal = refusal;

            return result;
        }
    }

    EquipResult TryEquipFromBag(InventoryComponent& bag, int bagSlot, LoadoutState& loadout, int targetSlot,
        const ItemCatalog& catalog)
    {
        const InventorySlot& carried = bag.GetSlot(bagSlot);
        if (carried.IsEmpty() || carried.item.effect.kind != ItemEffectKind::EquipWeapon)
        {
            return Refuse(EquipRefusal::NotAWeapon);
        }

        WeaponId id = WeaponId::Knife;
        if (!TryGetWeaponId(carried.item.effect.target, id))
        {
            return Refuse(EquipRefusal::UnknownWeapon);
        }

        int charge = carried.charge;
        int slot = targetSlot == NO_WEAPON_SLOT ? loadout.ResolveSlot(id) : targetSlot;

        if (slot < 0 || slot >= loadout.slotsCount)
        {
            return Refuse(EquipRefusal::NoSuchSlot);
        }

        if (!loadout.Fits(slot, id))
        {
            return Refuse(EquipRefusal::WrongKind);
        }

        if (!loadout.IsEmpty(slot) && loadout.slots[slot].id == id)
        {
            return Refuse(EquipRefusal::AlreadyThere);
        }

        EquipResult result;
        result.slot = slot;

        if (loadout.IsEmpty(slot))
        {
            bag.Remove(bagSlot, 1);
        }
        else
        {
            LoadoutSlot previous = loadout.slots[slot];

            // Ствол превращается обратно в предмет по своему же эффекту экипировки.
            const ItemDefinition* back = FindWeaponItem(catalog, GetWeapon(previous.id).id);
            if (back == nullptr)
            {
                return Refuse(EquipRefusal::NoWayBack);
            }

            bag.Replace(bagSlot, *back, previous.magazine);

            result.hasDisplaced = true;
            result.displaced = previous.id;
        }

        // Заряда нет у ствола, который ещё не был в руках: такому положен полный магазин.
        loadout.slots[slot].id = id;
        loadout.slots[slot].hasWeapon = true;
        loadout.slots[slot].magazine = charge >= 0 ? charge : GetWeapon(id).magazineSize;

        result.isDone = true;

        return result;
    }
}
