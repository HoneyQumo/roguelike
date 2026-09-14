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
        Gap,
        Blocked,
        Door
    };

    class LevelGrid
    {
    public:
        static LevelGrid Build(const LevelData& levelData);

        static const LevelGrid& Current();
        static void SetCurrent(LevelGrid grid);
        static void OpenCell(const XYZEngine::Vector2Df& position);

        int GetWidth() const;
        int GetHeight() const;
        bool IsEmpty() const;

        LevelCell GetCell(int column, int row) const;
        bool IsPassable(int column, int row) const;
        bool BlocksSight(int column, int row) const;

        XYZEngine::Vector2Df ToWorld(int column, int row) const;
        void ToCell(const XYZEngine::Vector2Df& position, int& column, int& row) const;

        bool HasWallBetween(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to) const;
        bool HasObstacleBetween(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to) const;
        bool FindFreeSpot(const XYZEngine::Vector2Df& position, XYZEngine::Vector2Df& spot) const;

    private:
        bool IsCrossed(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to, bool sightOnly) const;

        int width = 0;
        int height = 0;
        std::vector<LevelCell> cells;
    };
}
