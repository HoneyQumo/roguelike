#pragma once

#include "InventoryComponent.h"
#include "ItemCatalog.h"
#include "LoadoutRules.h"

namespace RoguelikeGame
{
    enum class EquipRefusal
    {
        None,
        NotAWeapon,
        UnknownWeapon,
        NoSuchSlot,
        WrongKind,
        AlreadyThere,
        NoWayBack
    };

    struct EquipResult
    {
        bool isDone = false;
        int slot = NO_WEAPON_SLOT;
        bool hasDisplaced = false;
        WeaponId displaced = WeaponId::Knife;
        EquipRefusal refusal = EquipRefusal::None;
    };

    /**
    *	Надеть ствол из сумки - это обмен, а не применение предмета.
    *
    *	Обмен идёт в той же ячейке: использованный ствол уходит, вытесненный встаёт
    *	на его место. Поэтому полная сумка обмену не мешает и терять оружие некуда.
    *
    *	Функция живёт отдельно от компонента снаряжения: у того за спиной анимация,
    *	звук и ресурсы, и в тестовый проект он не попадает.
    */
    EquipResult TryEquipFromBag(InventoryComponent& bag, int bagSlot, LoadoutState& loadout, int targetSlot,
        const ItemCatalog& catalog);
}
