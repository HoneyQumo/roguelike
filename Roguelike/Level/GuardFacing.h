#pragma once

#include <optional>
#include <MathUtils.h>
#include <Vector.h>
#include "GameSettings.h"
#include "LevelData.h"

namespace RoguelikeGame
{
    inline std::optional<XYZEngine::Vector2Df> FindTilePlace(const LevelData& levelData, TileType tile)
    {
        for (int row = 0; row < levelData.height; row++)
        {
            for (int column = 0; column < static_cast<int>(levelData.tiles[row].size()); column++)
            {
                if (levelData.tiles[row][column] == tile)
                {
                    return XYZEngine::Vector2Df{column * TILE_SIZE, (levelData.height - 1 - row) * TILE_SIZE};
                }
            }
        }

        return std::nullopt;
    }

    /**
    *	Куда развернуть караул на беговой локации: навстречу тому, кто бежит от входа
    *	к выходу. Стоять спиной к погоне заслону незачем.
    *
    *	На обычной карте угол не трогаем - там часовой сам решает, куда глядеть,
    *	и разворот сломал бы расчёт зон видимости, на котором держится скрытность.
    */
    inline std::optional<float> FacingAgainstTheRun(const LevelData& levelData)
    {
        if (levelData.pursuit.IsEmpty())
        {
            return std::nullopt;
        }

        std::optional<XYZEngine::Vector2Df> from = FindTilePlace(levelData, TileType::Entrance);
        if (!from.has_value())
        {
            from = FindTilePlace(levelData, TileType::PlayerSpawn);
        }

        std::optional<XYZEngine::Vector2Df> to = FindTilePlace(levelData, TileType::Exit);
        if (!to.has_value() && !levelData.escapes.empty())
        {
            // На беговой локации финиш - машина, тайла выхода там может и не быть.
            const FixturePlacement& car = levelData.escapes.front();
            to = XYZEngine::Vector2Df{car.column * TILE_SIZE, (levelData.height - 1 - car.row) * TILE_SIZE};
        }

        if (!from.has_value() || !to.has_value())
        {
            return std::nullopt;
        }

        XYZEngine::Vector2Df back = *from - *to;

        return back.IsZero() ? std::nullopt : std::optional<float>(XYZEngine::DegreesFromDirection(back.Normalized()));
    }
}
