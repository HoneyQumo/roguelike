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
        AddAmmo
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
        {"AddAmmo", ItemEffectKind::AddAmmo}
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
