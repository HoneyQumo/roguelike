#include "LevelGrid.h"
#include "GameSettings.h"
#include <algorithm>
#include <cmath>

namespace RoguelikeGame
{
    namespace
    {
        LevelGrid current;

        LevelCell CellOf(TileType tile)
        {
            if (tile == TileType::Wall)
            {
                return LevelCell::Wall;
            }

            return tile == TileType::Empty ? LevelCell::Gap : LevelCell::Floor;
        }
    }

    LevelGrid LevelGrid::Build(const LevelData& levelData)
    {
        LevelGrid grid;
        grid.height = static_cast<int>(levelData.tiles.size());
        grid.width = levelData.width;

        if (grid.width <= 0 || grid.height <= 0)
        {
            grid.width = 0;
            grid.height = 0;
            return grid;
        }

        grid.cells.assign(static_cast<std::size_t>(grid.width) * grid.height, LevelCell::Outside);

        for (int row = 0; row < grid.height; row++)
        {
            int columns = std::min(static_cast<int>(levelData.tiles[row].size()), grid.width);

            for (int column = 0; column < columns; column++)
            {
                grid.cells[static_cast<std::size_t>(row) * grid.width + column] = CellOf(levelData.tiles[row][column]);
            }
        }

        return grid;
    }

    const LevelGrid& LevelGrid::Current()
    {
        return current;
    }

    void LevelGrid::SetCurrent(LevelGrid grid)
    {
        current = std::move(grid);
    }

    int LevelGrid::GetWidth() const
    {
        return width;
    }

    int LevelGrid::GetHeight() const
    {
        return height;
    }

    bool LevelGrid::IsEmpty() const
    {
        return cells.empty();
    }

    LevelCell LevelGrid::GetCell(int column, int row) const
    {
        if (column < 0 || row < 0 || column >= width || row >= height)
        {
            return LevelCell::Outside;
        }

        return cells[static_cast<std::size_t>(row) * width + column];
    }

    bool LevelGrid::IsPassable(int column, int row) const
    {
        return GetCell(column, row) == LevelCell::Floor;
    }

    bool LevelGrid::BlocksSight(int column, int row) const
    {
        LevelCell cell = GetCell(column, row);

        return cell == LevelCell::Wall || cell == LevelCell::Outside;
    }

    XYZEngine::Vector2Df LevelGrid::ToWorld(int column, int row) const
    {
        return {column * TILE_SIZE, (height - 1 - row) * TILE_SIZE};
    }

    void LevelGrid::ToCell(const XYZEngine::Vector2Df& position, int& column, int& row) const
    {
        column = static_cast<int>(std::lround(position.x / TILE_SIZE));
        row = height - 1 - static_cast<int>(std::lround(position.y / TILE_SIZE));
    }

    bool LevelGrid::HasWallBetween(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to) const
    {
        if (IsEmpty())
        {
            return false;
        }

        int fromColumn = 0;
        int fromRow = 0;
        int toColumn = 0;
        int toRow = 0;
        ToCell(from, fromColumn, fromRow);
        ToCell(to, toColumn, toRow);

        int steps = std::max(std::abs(toColumn - fromColumn), std::abs(toRow - fromRow));
        if (steps <= 0)
        {
            return false;
        }

        for (int step = 1; step < steps; step++)
        {
            float part = static_cast<float>(step) / static_cast<float>(steps);
            int column = fromColumn + static_cast<int>(std::lround((toColumn - fromColumn) * part));
            int row = fromRow + static_cast<int>(std::lround((toRow - fromRow) * part));

            if (BlocksSight(column, row))
            {
                return true;
            }
        }

        return false;
    }
}
