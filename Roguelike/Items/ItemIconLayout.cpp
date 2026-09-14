#include "ItemIconLayout.h"
#include "GameSettings.h"
#include <algorithm>

namespace RoguelikeGame
{
    sf::Vector2f IconWorldSize(const ItemIcon& icon)
    {
        float width = static_cast<float>(icon.rect.width);
        float height = static_cast<float>(icon.rect.height);
        float longest = std::max(width, height);

        if (longest <= 0.f)
        {
            return {0.f, 0.f};
        }

        float scale = ITEM_WORLD_SIZE / longest * icon.worldScale;

        return {width * scale, height * scale};
    }
}
