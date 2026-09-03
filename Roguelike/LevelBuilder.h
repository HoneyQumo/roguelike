#pragma once

#include "Level.h"
#include "LevelData.h"
#include <Vector.h>

namespace RoguelikeGame
{
    class LevelBuilder
    {
    public:
        static Level Build(const LevelData& levelData);

    private:
        static int BuildTiles(const LevelData& levelData, Level& level);
        static XYZEngine::Vector2Df TileToWorldPosition(int column, int row, int levelHeight);
    };
}
