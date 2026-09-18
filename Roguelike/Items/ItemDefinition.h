#pragma once

#include <string>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace RoguelikeGame
{
    enum class ItemType
    {
        Consumable,
        Key,
        Weapon
    };

    enum class ItemEffectKind
    {
        None,
        Heal,
        Unlock,
        EquipWeapon,
        AddAmmo,
        AddArmor
    };

    struct ItemTypeName
    {
        const char* name;
        ItemType type;
    };

    struct ItemEffectName
    {
        const char* name;
        ItemEffectKind kind;
    };

    constexpr ItemTypeName ITEM_TYPE_NAMES[] = {
        {"Consumable", ItemType::Consumable},
        {"Key", ItemType::Key},
        {"Weapon", ItemType::Weapon}
    };

    constexpr ItemEffectName ITEM_EFFECT_NAMES[] = {
        {"None", ItemEffectKind::None},
        {"Heal", ItemEffectKind::Heal},
        {"Unlock", ItemEffectKind::Unlock},
        {"EquipWeapon", ItemEffectKind::EquipWeapon},
        {"AddAmmo", ItemEffectKind::AddAmmo},
        {"AddArmor", ItemEffectKind::AddArmor}
    };

    struct ItemIcon
    {
        std::string texturePath;
        sf::IntRect rect;
        sf::Color tint = sf::Color::White;
        float worldScale = 1.f;
    };

    struct ItemEffect
    {
        ItemEffectKind kind = ItemEffectKind::None;
        float amount = 0.f;
        std::string target;
    };

    enum class ItemRefuseReason
    {
        None,
        NoHandler,
        NotNeeded,
        Unknown
    };

    /**
    *	Ответ на попытку применить предмет: применилось ли, сколько списать и почему нет.
    *
    *	Сколько тратить, решает сам эффект, а не сумка: иначе новый вид эффекта
    *	каждый раз требовал бы правки инвентаря.
    *
    *	Превращение из bool неявное нарочно: обработчику, которому причина не нужна,
    *	по-прежнему хватает да или нет, и ни одна старая регистрация не переписывается.
    */
    struct ItemUseResult
    {
        bool isApplied = false;
        int consumed = 0;
        ItemRefuseReason reason = ItemRefuseReason::None;

        ItemUseResult() = default;

        constexpr ItemUseResult(bool isDone)
            : isApplied(isDone), consumed(isDone ? 1 : 0),
              reason(isDone ? ItemRefuseReason::None : ItemRefuseReason::NotNeeded)
        {
        }

        constexpr ItemUseResult(bool isDone, int taken, ItemRefuseReason why)
            : isApplied(isDone), consumed(taken), reason(why)
        {
        }
    };

    struct ItemDefinition
    {
        std::string id;
        std::string name;
        ItemType type = ItemType::Consumable;
        ItemIcon icon;
        bool stackable = false;
        int maxStack = 1;
        ItemEffect effect;
    };
}
