#pragma once

#include <string>
#include <vector>
#include <Vector.h>
#include "LevelData.h"
#include "LevelGrid.h"

namespace RoguelikeGame
{
    struct PatrolRoute
    {
        std::string id;
        std::vector<XYZEngine::Vector2Df> points;
    };

    class PatrolRoutes
    {
    public:
        static PatrolRoutes Build(const LevelData& levelData, const LevelGrid& grid);

        static const PatrolRoutes& Current();
        static void SetCurrent(PatrolRoutes routes);

        bool IsEmpty() const;
        std::size_t GetCount() const;
        const PatrolRoute* Find(const std::string& id) const;
        const PatrolRoute* Nearest(const XYZEngine::Vector2Df& position, float maxDistance) const;

    private:
        std::vector<PatrolRoute> routes;
    };
}
