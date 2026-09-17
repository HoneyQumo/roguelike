#pragma once

#include <Vector.h>
#include "LevelGrid.h"
#include "PathField.h"

namespace RoguelikeGame
{
    /**
    *	Ищет ближайшую клетку, с которой угроза врага не видит.
    *
    *	`field` строится от самого врага, поэтому `GetDistance` - цена хода:
    *	побеждает ближайшее укрытие, а не самое надёжное. Перезарядка коротка,
    *	бежать через полкарты незачем.
    */
    bool FindCoverSpot(const LevelGrid& grid, const PathField& field, const XYZEngine::Vector2Df& from,
        const XYZEngine::Vector2Df& threat, int radius, XYZEngine::Vector2Df& spot);
}
