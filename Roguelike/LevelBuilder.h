#pragma once

#include "ItemCatalog.h"
#include "Level.h"
#include "LevelData.h"
#include <Vector.h>

namespace RoguelikeGame
{
    class LevelBuilder
    {
    public:
        static Level Build(const LevelData& levelData, const ItemCatalog& items = ItemCatalog::Empty());

    private:
        static int BuildTiles(const LevelData& levelData, Level& level);
        static int BuildItems(const LevelData& levelData, const ItemCatalog& items, Level& level);
        static XYZEngine::Vector2Df TileToWorldPosition(int column, int row, int levelHeight);
    };
}
