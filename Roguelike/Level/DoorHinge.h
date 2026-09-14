#pragma once

#include <Vector.h>

namespace RoguelikeGame
{
    class LevelGrid;

    struct DoorHinge
    {
        XYZEngine::Vector2Df pivot;
        float closedAngle = 0.f;
        float openAngle = -90.f;
    };

    float LeafAngleFor(const XYZEngine::Vector2Df& direction);
    DoorHinge HingeFor(const LevelGrid& grid, int column, int row);
}
