#pragma once

#include <string>
#include <vector>
#include "ItemDefinition.h"

namespace RoguelikeGame
{
    inline bool IsKeyFor(const ItemDefinition& item, const std::string& doorId)
    {
        return item.effect.kind == ItemEffectKind::Unlock && !doorId.empty() && item.effect.target == doorId;
    }

    inline const ItemDefinition* FindKeyFor(const std::vector<const ItemDefinition*>& carried, const std::string& doorId)
    {
        for (const ItemDefinition* item : carried)
        {
            if (item != nullptr && IsKeyFor(*item, doorId))
            {
                return item;
            }
        }

        return nullptr;
    }
}
