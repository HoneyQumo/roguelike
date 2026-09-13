#include "LootDrop.h"
#include "GameResources.h"
#include "GameSettings.h"
#include "Item.h"
#include "ItemCatalog.h"
#include "LootCatalog.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <MathUtils.h>
#include <LoggerRegistry.h>
#include <randomizer.h>

namespace RoguelikeGame
{
    namespace
    {
        int RollWeight(int total)
        {
            return total <= 1 ? 0 : random<int>(0, total - 1);
        }

        XYZEngine::Vector2Df SpreadAround(const XYZEngine::Vector2Df& position, int index, int count)
        {
            float step = count > 0 ? 360.f / count : 360.f;
            float angle = random<float>(0.f, step) + step * index;

            return position + XYZEngine::RotateByDegrees({1.f, 0.f}, angle) * LOOT_DROP_SPREAD;
        }
    }

    void DropLoot(const std::string& tableId, XYZEngine::GameObject* owner, const XYZEngine::Vector2Df& position,
        const LootCatalog& loot, const ItemCatalog& items, int (*roll)(int))
    {
        const LootTable* table = loot.Find(tableId);
        if (table == nullptr)
        {
            if (!tableId.empty())
            {
                LOG_ERROR("Unknown loot table: " + tableId);
            }

            return;
        }

        int total = table->GetTotalWeight();
        int dropped = 0;

        for (int index = 0; index < table->rolls; index++)
        {
            LootDrop drop = table->Pick(roll(total));
            if (drop.IsEmpty())
            {
                continue;
            }

            const ItemDefinition* definition = items.Find(drop.itemId);
            if (definition == nullptr)
            {
                LOG_ERROR("Loot table " + tableId + " asks for unknown item " + drop.itemId);
                continue;
            }

            try
            {
                XYZEngine::Vector2Df place = SpreadAround(position, index, table->rolls);
                if (CreateItem(*definition, place, owner) != nullptr)
                {
                    dropped++;
                }
            }
            catch (const std::exception& exception)
            {
                LOG_ERROR("Loot item is not created: " + drop.itemId + ", " + exception.what());
            }
        }

        if (dropped > 0)
        {
            LOG_INFO("Loot from " + tableId + ": " + std::to_string(dropped) + " item(s)");
        }
    }

    void DropLoot(const std::string& tableId, XYZEngine::GameObject* owner, const XYZEngine::Vector2Df& position)
    {
        DropLoot(tableId, owner, position, GameResources::GetLoot(), GameResources::GetItems(), RollWeight);
    }
}
