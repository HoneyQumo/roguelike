#pragma once

#include <array>
#include <Vector.h>
#include "PathField.h"

namespace RoguelikeGame
{
    constexpr int PATH_FIELD_SLOTS = 4;

    class PathService
    {
    public:
        static PathService& Current();
        static void Reset();

        const PathField* FieldTo(const XYZEngine::Vector2Df& goalPosition);
        bool RouteTo(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& goalPosition,
            std::vector<XYZEngine::Vector2Df>& route);

        void Clear();
        int GetBuildCount() const;

    private:
        struct Slot
        {
            PathField field;
            int column = 0;
            int row = 0;
            unsigned int usedAt = 0u;
            bool isFilled = false;
        };

        Slot* Take(int column, int row);

        std::array<Slot, PATH_FIELD_SLOTS> slots;
        unsigned int clock = 0u;
        int buildCount = 0;
    };
}
