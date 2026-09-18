#include "FogOfWar.h"
#include "LevelGrid.h"
#include "SightRules.h"

namespace RoguelikeGame
{
    FogOfWar FogOfWar::current;

    FogOfWar& FogOfWar::Current()
    {
        return current;
    }

    void FogOfWar::Reset(int width, int height, int radius)
    {
        current.width = width > 0 ? width : 0;
        current.height = height > 0 ? height : 0;
        current.radius = radius > 0 ? radius : 0;
        current.version++;

        current.cells.assign(static_cast<std::size_t>(current.width) * current.height, FogState::Unseen);
    }

    bool FogOfWar::IsEnabled() const
    {
        return radius > 0 && width > 0 && height > 0;
    }

    int FogOfWar::GetWidth() const
    {
        return width;
    }

    int FogOfWar::GetHeight() const
    {
        return height;
    }

    int FogOfWar::GetRadius() const
    {
        return radius;
    }

    unsigned int FogOfWar::GetVersion() const
    {
        return version;
    }

    FogState FogOfWar::GetState(int column, int row) const
    {
        if (!IsEnabled())
        {
            return FogState::Seen;
        }

        if (column < 0 || row < 0 || column >= width || row >= height)
        {
            return FogState::Unseen;
        }

        return cells[static_cast<std::size_t>(row) * width + column];
    }

    FogState FogOfWar::GetStateAt(const XYZEngine::Vector2Df& position) const
    {
        if (!IsEnabled())
        {
            return FogState::Seen;
        }

        int column = 0;
        int row = 0;
        LevelGrid::Current().ToCell(position, column, row);

        return GetState(column, row);
    }

    int FogOfWar::Count(FogState state) const
    {
        int total = 0;

        for (FogState cell : cells)
        {
            total += cell == state ? 1 : 0;
        }

        return total;
    }

    bool FogOfWar::Reveal(const LevelGrid& grid, const XYZEngine::Vector2Df& from)
    {
        if (!IsEnabled())
        {
            return false;
        }

        int column = 0;
        int row = 0;
        grid.ToCell(from, column, row);

        std::vector<FogState> before = cells;

        // Видимое сначала гаснет до памяти: обзор считается заново целиком,
        // иначе клетка, которую заслонили, осталась бы видимой навсегда.
        for (FogState& cell : cells)
        {
            if (cell == FogState::Seen)
            {
                cell = FogState::Known;
            }
        }

        for (int cellRow = row - radius; cellRow <= row + radius; cellRow++)
        {
            for (int cellColumn = column - radius; cellColumn <= column + radius; cellColumn++)
            {
                if (cellColumn < 0 || cellRow < 0 || cellColumn >= width || cellRow >= height)
                {
                    continue;
                }

                int alongColumns = cellColumn - column;
                int alongRows = cellRow - row;
                if (alongColumns * alongColumns + alongRows * alongRows > radius * radius)
                {
                    continue;
                }

                if (grid.HasWallBetween(from, grid.ToWorld(cellColumn, cellRow)))
                {
                    continue;
                }

                cells[static_cast<std::size_t>(cellRow) * width + cellColumn] = FogState::Seen;
            }
        }

        LightBlockers(grid, column, row);

        if (cells == before)
        {
            return false;
        }

        version++;

        return true;
    }

    /**
    *	Луч до центра клетки-стены под косым углом задевает соседнюю стену и объявляет
    *	цель закрытой - поэтому стены заслоняли друг друга и в радиусе светилась четверть.
    *	Второй проход досвечивает их по уже посчитанному полу: стена видна, если виден
    *	её сосед со стороны игрока.
    */
    void FogOfWar::LightBlockers(const LevelGrid& grid, int fromColumn, int fromRow)
    {
        std::vector<std::size_t> lit;

        for (int cellRow = fromRow - radius; cellRow <= fromRow + radius; cellRow++)
        {
            for (int cellColumn = fromColumn - radius; cellColumn <= fromColumn + radius; cellColumn++)
            {
                if (cellColumn < 0 || cellRow < 0 || cellColumn >= width || cellRow >= height)
                {
                    continue;
                }

                int alongColumns = cellColumn - fromColumn;
                int alongRows = cellRow - fromRow;
                if (alongColumns * alongColumns + alongRows * alongRows > radius * radius)
                {
                    continue;
                }

                std::size_t index = static_cast<std::size_t>(cellRow) * width + cellColumn;
                if (cells[index] == FogState::Seen || !grid.BlocksSight(cellColumn, cellRow))
                {
                    continue;
                }

                SightStep steps[2];
                int count = StepsTowardViewer(alongColumns, alongRows, steps);

                for (int step = 0; step < count; step++)
                {
                    int neighbourColumn = cellColumn + steps[step].column;
                    int neighbourRow = cellRow + steps[step].row;

                    if (grid.BlocksSight(neighbourColumn, neighbourRow)
                        || GetState(neighbourColumn, neighbourRow) != FogState::Seen)
                    {
                        continue;
                    }

                    lit.push_back(index);
                    break;
                }
            }
        }

        for (std::size_t index : lit)
        {
            cells[index] = FogState::Seen;
        }
    }
}
