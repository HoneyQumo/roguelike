#include "PathService.h"
#include "LevelGrid.h"

using namespace XYZEngine;

namespace RoguelikeGame
{
    namespace
    {
        PathService service;
    }

    PathService& PathService::Current()
    {
        return service;
    }

    void PathService::Reset()
    {
        service.Clear();
    }

    void PathService::Clear()
    {
        for (Slot& slot : slots)
        {
            slot.field.Clear();
            slot.isFilled = false;
            slot.usedAt = 0u;
        }

        clock = 0u;
        buildCount = 0;
    }

    PathService::Slot* PathService::Take(int column, int row)
    {
        for (Slot& slot : slots)
        {
            if (slot.isFilled && slot.column == column && slot.row == row)
            {
                slot.usedAt = ++clock;
                return &slot;
            }
        }

        Slot* oldest = &slots[0];
        for (Slot& slot : slots)
        {
            if (!slot.isFilled)
            {
                oldest = &slot;
                break;
            }

            if (slot.usedAt < oldest->usedAt)
            {
                oldest = &slot;
            }
        }

        oldest->column = column;
        oldest->row = row;
        oldest->usedAt = ++clock;
        oldest->isFilled = true;
        oldest->field.Build(LevelGrid::Current(), column, row);
        buildCount++;

        return oldest;
    }

    const PathField* PathService::FieldTo(const Vector2Df& goalPosition)
    {
        const LevelGrid& grid = LevelGrid::Current();
        if (grid.IsEmpty())
        {
            return nullptr;
        }

        int column = 0;
        int row = 0;
        grid.ToCell(goalPosition, column, row);

        Slot* slot = Take(column, row);

        return slot->field.IsEmpty() ? nullptr : &slot->field;
    }

    bool PathService::RouteTo(const Vector2Df& from, const Vector2Df& goalPosition, std::vector<Vector2Df>& route)
    {
        route.clear();

        const PathField* field = FieldTo(goalPosition);
        if (field == nullptr)
        {
            return false;
        }

        const LevelGrid& grid = LevelGrid::Current();
        int column = 0;
        int row = 0;
        grid.ToCell(from, column, row);

        return field->BuildRoute(grid, column, row, route);
    }

    int PathService::GetBuildCount() const
    {
        return buildCount;
    }
}
