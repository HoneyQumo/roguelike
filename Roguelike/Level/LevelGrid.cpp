#include "LevelGrid.h"
#include "GameSettings.h"
#include <algorithm>
#include <cmath>

namespace RoguelikeGame
{
    namespace
    {
        LevelGrid current;

        constexpr float FAR_AHEAD = 1e9f;

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

        for (const PropPlacement& prop : levelData.props)
        {
            if (prop.column < 0 || prop.row < 0 || prop.column >= grid.width || prop.row >= grid.height)
            {
                continue;
            }

            std::size_t index = static_cast<std::size_t>(prop.row) * grid.width + prop.column;
            if (grid.cells[index] == LevelCell::Floor)
            {
                grid.cells[index] = LevelCell::Blocked;
            }
        }

        return grid;
    }

    void LevelGrid::OpenCell(const XYZEngine::Vector2Df& position)
    {
        int column = 0;
        int row = 0;
        current.ToCell(position, column, row);

        if (column < 0 || row < 0 || column >= current.width || row >= current.height)
        {
            return;
        }

        std::size_t index = static_cast<std::size_t>(row) * current.width + column;
        if (current.cells[index] == LevelCell::Blocked)
        {
            current.cells[index] = LevelCell::Floor;
        }
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
        return IsCrossed(from, to, true);
    }

    bool LevelGrid::HasObstacleBetween(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to) const
    {
        return IsCrossed(from, to, false);
    }

    bool LevelGrid::IsCrossed(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to, bool sightOnly) const
    {
        if (IsEmpty())
        {
            return false;
        }

        float fromU = from.x / TILE_SIZE + 0.5f;
        float fromV = height - 0.5f - from.y / TILE_SIZE;
        float toU = to.x / TILE_SIZE + 0.5f;
        float toV = height - 0.5f - to.y / TILE_SIZE;

        int column = static_cast<int>(std::floor(fromU));
        int row = static_cast<int>(std::floor(fromV));
        int lastColumn = static_cast<int>(std::floor(toU));
        int lastRow = static_cast<int>(std::floor(toV));

        float spanU = toU - fromU;
        float spanV = toV - fromV;

        int stepColumn = spanU > 0.f ? 1 : -1;
        int stepRow = spanV > 0.f ? 1 : -1;

        float nextColumn = spanU != 0.f
            ? ((spanU > 0.f ? column + 1 - fromU : fromU - column) / std::abs(spanU))
            : FAR_AHEAD;
        float nextRow = spanV != 0.f
            ? ((spanV > 0.f ? row + 1 - fromV : fromV - row) / std::abs(spanV))
            : FAR_AHEAD;

        float overColumn = spanU != 0.f ? 1.f / std::abs(spanU) : FAR_AHEAD;
        float overRow = spanV != 0.f ? 1.f / std::abs(spanV) : FAR_AHEAD;

        int guard = std::abs(lastColumn - column) + std::abs(lastRow - row) + 2;

        for (int step = 0; step < guard; step++)
        {
            if (column == lastColumn && row == lastRow)
            {
                return false;
            }

            if (nextColumn <= nextRow)
            {
                column += stepColumn;
                nextColumn += overColumn;
            }
            else
            {
                row += stepRow;
                nextRow += overRow;
            }

            if (column == lastColumn && row == lastRow)
            {
                return false;
            }

            bool isBlocking = sightOnly ? BlocksSight(column, row) : !IsPassable(column, row);
            if (isBlocking)
            {
                return true;
            }
        }

        return false;
    }
}
