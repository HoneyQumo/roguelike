#include "FogOfWar.h"
#include "LevelGrid.h"
#include "GameSettings.h"
#include "Shadowcast.h"
#include <algorithm>
#include <cmath>

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
        current.light.assign(current.cells.size(), 0.f);
        current.seen.clear();
        current.seenBefore.clear();
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

        // Гаснет только то, что было видно в прошлый раз, а не вся карта:
        // на мосту её в сотни тайлов, а видно за раз сотню клеток.
        seenBefore = seen;

        for (std::size_t index : seenBefore)
        {
            cells[index] = FogState::Known;
            light[index] = FOG_KNOWN_LIGHT;
        }

        seen.clear();

        float full = FOG_FULL_PART * radius;
        float span = std::max(radius - full, 1.f);

        SightOrigin origin;
        origin.column = column;
        origin.row = row;
        origin.radius = radius;

        Shadowcast(origin,
            [&grid](int cellColumn, int cellRow) { return grid.BlocksSight(cellColumn, cellRow); },
            [&](int cellColumn, int cellRow)
            {
                if (cellColumn < 0 || cellRow < 0 || cellColumn >= width || cellRow >= height)
                {
                    return;
                }

                std::size_t index = static_cast<std::size_t>(cellRow) * width + cellColumn;
                if (cells[index] == FogState::Seen)
                {
                    return;
                }

                cells[index] = FogState::Seen;
                seen.push_back(index);

                float alongColumns = static_cast<float>(cellColumn - column);
                float alongRows = static_cast<float>(cellRow - row);
                float distance = std::sqrt(alongColumns * alongColumns + alongRows * alongRows);
                float part = std::clamp((distance - full) / span, 0.f, 1.f);

                light[index] = std::max(FOG_KNOWN_LIGHT, 1.f - part * (1.f - FOG_EDGE_LIGHT));
            });

        std::sort(seen.begin(), seen.end());

        if (seen == seenBefore)
        {
            return false;
        }

        version++;

        return true;
    }

    float FogOfWar::GetLight(int column, int row) const
    {
        if (!IsEnabled())
        {
            return 1.f;
        }

        if (column < 0 || row < 0 || column >= width || row >= height)
        {
            return 0.f;
        }

        return light[static_cast<std::size_t>(row) * width + column];
    }

    float FogOfWar::GetCornerLight(int column, int row) const
    {
        if (!IsEnabled())
        {
            return 1.f;
        }

        // Угол с номером клетки - это её левый верхний: соседи лежат слева и сверху.
        return 0.25f * (GetLight(column - 1, row - 1) + GetLight(column, row - 1)
            + GetLight(column - 1, row) + GetLight(column, row));
    }
}
