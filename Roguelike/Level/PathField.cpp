#include "PathField.h"

namespace RoguelikeGame
{
    namespace
    {
        constexpr int STEP_COLUMNS[4] = {1, -1, 0, 0};
        constexpr int STEP_ROWS[4] = {0, 0, 1, -1};
    }

    void PathField::Clear()
    {
        width = 0;
        height = 0;
        goalColumn = 0;
        goalRow = 0;
        distances.clear();
    }

    void PathField::Build(const LevelGrid& grid, int newGoalColumn, int newGoalRow)
    {
        Clear();

        if (grid.GetWidth() <= 0 || grid.GetHeight() <= 0 || !grid.IsPassable(newGoalColumn, newGoalRow))
        {
            return;
        }

        width = grid.GetWidth();
        height = grid.GetHeight();
        goalColumn = newGoalColumn;
        goalRow = newGoalRow;
        distances.assign(static_cast<std::size_t>(width) * height, PATH_UNREACHABLE);

        std::vector<int> wave;
        wave.reserve(distances.size());

        int goalIndex = goalRow * width + goalColumn;
        distances[goalIndex] = 0;
        wave.push_back(goalIndex);

        for (std::size_t head = 0; head < wave.size(); head++)
        {
            int index = wave[head];
            int column = index % width;
            int row = index / width;
            int step = distances[index] + 1;

            for (int side = 0; side < 4; side++)
            {
                int neighbourColumn = column + STEP_COLUMNS[side];
                int neighbourRow = row + STEP_ROWS[side];

                if (!grid.IsPassable(neighbourColumn, neighbourRow))
                {
                    continue;
                }

                int neighbourIndex = neighbourRow * width + neighbourColumn;
                if (distances[neighbourIndex] != PATH_UNREACHABLE)
                {
                    continue;
                }

                distances[neighbourIndex] = step;
                wave.push_back(neighbourIndex);
            }
        }
    }

    bool PathField::IsEmpty() const
    {
        return distances.empty();
    }

    int PathField::GetGoalColumn() const
    {
        return goalColumn;
    }

    int PathField::GetGoalRow() const
    {
        return goalRow;
    }

    int PathField::GetDistance(int column, int row) const
    {
        if (column < 0 || row < 0 || column >= width || row >= height)
        {
            return PATH_UNREACHABLE;
        }

        return distances[static_cast<std::size_t>(row) * width + column];
    }

    bool PathField::IsReachable(int column, int row) const
    {
        return GetDistance(column, row) != PATH_UNREACHABLE;
    }

    bool PathField::NextCell(int column, int row, int& nextColumn, int& nextRow) const
    {
        int here = GetDistance(column, row);
        if (here <= 0)
        {
            return false;
        }

        int best = here;
        bool isFound = false;

        for (int side = 0; side < 4; side++)
        {
            int neighbourColumn = column + STEP_COLUMNS[side];
            int neighbourRow = row + STEP_ROWS[side];
            int neighbour = GetDistance(neighbourColumn, neighbourRow);

            if (neighbour == PATH_UNREACHABLE || neighbour >= best)
            {
                continue;
            }

            best = neighbour;
            nextColumn = neighbourColumn;
            nextRow = neighbourRow;
            isFound = true;
        }

        return isFound;
    }

    bool PathField::BuildRoute(const LevelGrid& grid, int fromColumn, int fromRow, std::vector<XYZEngine::Vector2Df>& route) const
    {
        route.clear();

        if (!IsReachable(fromColumn, fromRow))
        {
            return false;
        }

        int column = fromColumn;
        int row = fromRow;

        for (std::size_t step = 0; step < distances.size(); step++)
        {
            int nextColumn = column;
            int nextRow = row;

            if (!NextCell(column, row, nextColumn, nextRow))
            {
                break;
            }

            column = nextColumn;
            row = nextRow;
            route.push_back(grid.ToWorld(column, row));
        }

        return !route.empty();
    }
}
