#pragma once

#include <vector>
#include <Vector.h>
#include "LevelGrid.h"

namespace RoguelikeGame
{
    constexpr int PATH_UNREACHABLE = -1;

    class PathField
    {
    public:
        void Build(const LevelGrid& grid, int newGoalColumn, int newGoalRow);
        void Clear();

        bool IsEmpty() const;
        int GetGoalColumn() const;
        int GetGoalRow() const;

        int GetDistance(int column, int row) const;
        bool IsReachable(int column, int row) const;

        bool NextCell(int column, int row, int& nextColumn, int& nextRow) const;
        bool BuildRoute(const LevelGrid& grid, int fromColumn, int fromRow, std::vector<XYZEngine::Vector2Df>& route) const;

    private:
        int width = 0;
        int height = 0;
        int goalColumn = 0;
        int goalRow = 0;
        std::vector<int> distances;
    };
}
