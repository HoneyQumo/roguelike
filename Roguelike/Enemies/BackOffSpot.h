#pragma once

#include <Vector.h>
#include "LevelGrid.h"
#include "PathField.h"

namespace RoguelikeGame
{
    /**
    *	Ищет, куда отойти от угрозы, чтобы не упереться спиной в стену.
    *
    *	`field` строится от самого врага, поэтому `GetDistance` - честная цена
    *	хода, а не расстояние по прямой. Дальше комфортной дистанции выигрыша
    *	нет: отходить через всю карту незачем.
    */
    bool FindBackOffSpot(const LevelGrid& grid, const PathField& field, const XYZEngine::Vector2Df& from,
        const XYZEngine::Vector2Df& threat, float wanted, int radius, XYZEngine::Vector2Df& spot);
}
