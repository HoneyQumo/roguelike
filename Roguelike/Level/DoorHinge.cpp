#include "DoorHinge.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include <MathUtils.h>

namespace RoguelikeGame
{
    namespace
    {
        bool IsWallLike(const LevelGrid& grid, int column, int row)
        {
            LevelCell cell = grid.GetCell(column, row);

            return cell == LevelCell::Wall || cell == LevelCell::Outside;
        }

        float TurnSide(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to)
        {
            return from.x * to.y - from.y * to.x >= 0.f ? DOOR_SWING_ANGLE : -DOOR_SWING_ANGLE;
        }
    }

    float LeafAngleFor(const XYZEngine::Vector2Df& direction)
    {
        return XYZEngine::DegreesFromDirection(direction) - 90.f;
    }

    DoorHinge HingeFor(const LevelGrid& grid, int column, int row)
    {
        bool isBlockedByRow = !grid.IsPassable(column, row - 1) && !grid.IsPassable(column, row + 1);
        bool isBlockedByColumn = !grid.IsPassable(column - 1, row) && !grid.IsPassable(column + 1, row);
        bool isStanding = isBlockedByRow || !isBlockedByColumn;

        XYZEngine::Vector2Df along = isStanding ? XYZEngine::Vector2Df{0.f, 1.f} : XYZEngine::Vector2Df{1.f, 0.f};
        XYZEngine::Vector2Df across = isStanding ? XYZEngine::Vector2Df{1.f, 0.f} : XYZEngine::Vector2Df{0.f, 1.f};

        int alongColumn = isStanding ? 0 : 1;
        int alongRow = isStanding ? -1 : 0;
        int acrossColumn = isStanding ? 1 : 0;
        int acrossRow = isStanding ? 0 : -1;

        bool isHeldAhead = IsWallLike(grid, column + alongColumn, row + alongRow)
            && !IsWallLike(grid, column - alongColumn, row - alongRow);
        bool isFreeAhead = grid.IsPassable(column + acrossColumn, row + acrossRow)
            || !grid.IsPassable(column - acrossColumn, row - acrossRow);

        XYZEngine::Vector2Df leaf = isHeldAhead ? -along : along;
        XYZEngine::Vector2Df swing = isFreeAhead ? across : -across;

        DoorHinge hinge;
        hinge.pivot = grid.ToWorld(column, row) - leaf * (0.5f * TILE_SIZE);
        hinge.closedAngle = LeafAngleFor(leaf);
        hinge.openAngle = hinge.closedAngle + TurnSide(leaf, swing);

        return hinge;
    }
}
