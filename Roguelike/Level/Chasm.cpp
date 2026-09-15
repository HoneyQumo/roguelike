#include "Chasm.h"

namespace RoguelikeGame
{
    namespace
    {
        bool IsWalkable(TileType tile)
        {
            return tile != TileType::Empty && tile != TileType::Wall;
        }
    }

    bool IsChasmEdge(const LevelData& levelData, int column, int row)
    {
        if (TileAt(levelData, column, row) != TileType::Empty)
        {
            return false;
        }

        const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
        for (const auto& step : steps)
        {
            if (IsWalkable(TileAt(levelData, column + step[0], row + step[1])))
            {
                return true;
            }
        }

        return false;
    }
}
