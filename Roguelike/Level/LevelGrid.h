#pragma once

#include <vector>
#include <Vector.h>
#include "LevelData.h"

namespace RoguelikeGame
{
    enum class LevelCell : unsigned char
    {
        Outside,
        Floor,
        Wall,
        Gap
    };

    class LevelGrid
    {
    public:
        static LevelGrid Build(const LevelData& levelData);

        static const LevelGrid& Current();
        static void SetCurrent(LevelGrid grid);

        int GetWidth() const;
        int GetHeight() const;
        bool IsEmpty() const;

        LevelCell GetCell(int column, int row) const;
        bool IsPassable(int column, int row) const;
        bool BlocksSight(int column, int row) const;

        XYZEngine::Vector2Df ToWorld(int column, int row) const;
        void ToCell(const XYZEngine::Vector2Df& position, int& column, int& row) const;

        bool HasWallBetween(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to) const;

    private:
        int width = 0;
        int height = 0;
        std::vector<LevelCell> cells;
    };
}
