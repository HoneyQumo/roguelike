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
        Screen,
        Brush,
        Door
    };

    class PropCatalog;

    class LevelGrid
    {
    public:
        static LevelGrid Build(const LevelData& levelData);
        static LevelGrid Build(const LevelData& levelData, const PropCatalog& props);

        static const LevelGrid& Current();
        static void SetCurrent(LevelGrid grid);
        static void OpenCell(const XYZEngine::Vector2Df& position);

        int GetWidth() const;
        int GetHeight() const;
        bool IsEmpty() const;

        // Номер правки: открытая дверь меняет линию видимости, и туман пересчитывается.
        unsigned int GetVersion() const;

        LevelCell GetCell(int column, int row) const;
        bool IsPassable(int column, int row) const;
        bool BlocksSight(int column, int row) const;
        bool BlocksSound(int column, int row) const;

        XYZEngine::Vector2Df ToWorld(int column, int row) const;
        void ToCell(const XYZEngine::Vector2Df& position, int& column, int& row) const;

        bool HasWallBetween(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to) const;
        bool HasObstacleBetween(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to) const;
        bool FindFreeSpot(const XYZEngine::Vector2Df& position, XYZEngine::Vector2Df& spot) const;
        int CountWallsBetween(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to) const;

    private:
        enum class CrossKind : unsigned char
        {
            Sight,
            Move,
            Sound
        };

        bool IsCrossed(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to, CrossKind kind) const;
        int CountCrossed(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to, CrossKind kind, bool stopAtFirst) const;

        int width = 0;
        int height = 0;
        unsigned int version = 0u;
        std::vector<LevelCell> cells;
    };
}
