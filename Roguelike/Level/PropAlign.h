#pragma once

#include <string>
#include "LevelData.h"

namespace RoguelikeGame
{
    constexpr float PANEL_UPRIGHT_ANGLE = 90.f;

    struct PanelSupport
    {
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
    };

    /**
    *	Панель встаёт вдоль той линии, где у неё больше опоры. Поровну - лежит
    *	горизонтально, как нарисована.
    */
    constexpr float PanelAngle(const PanelSupport& support)
    {
        int upright = (support.up ? 1 : 0) + (support.down ? 1 : 0);
        int flat = (support.left ? 1 : 0) + (support.right ? 1 : 0);

        return upright > flat ? PANEL_UPRIGHT_ANGLE : 0.f;
    }

    inline bool HasPropAt(const LevelData& levelData, const std::string& propId, int column, int row)
    {
        for (const PropPlacement& placement : levelData.props)
        {
            if (placement.column == column && placement.row == row && placement.propId == propId)
            {
                return true;
            }
        }

        return false;
    }

    inline bool IsPanelSupport(const LevelData& levelData, const std::string& propId, int column, int row)
    {
        TileType tile = TileAt(levelData, column, row);

        return tile == TileType::Wall || tile == TileType::Empty || HasPropAt(levelData, propId, column, row);
    }

    inline PanelSupport ReadPanelSupport(const LevelData& levelData, const std::string& propId, int column, int row)
    {
        PanelSupport support;
        support.up = IsPanelSupport(levelData, propId, column, row - 1);
        support.down = IsPanelSupport(levelData, propId, column, row + 1);
        support.left = IsPanelSupport(levelData, propId, column - 1, row);
        support.right = IsPanelSupport(levelData, propId, column + 1, row);

        return support;
    }
}
