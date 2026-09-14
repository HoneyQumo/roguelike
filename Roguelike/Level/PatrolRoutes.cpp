#include "PatrolRoutes.h"
#include <algorithm>

using namespace XYZEngine;

namespace RoguelikeGame
{
    namespace
    {
        PatrolRoutes current;
    }

    PatrolRoutes PatrolRoutes::Build(const LevelData& levelData, const LevelGrid& grid)
    {
        std::vector<PatrolPoint> ordered = levelData.patrols;
        std::stable_sort(ordered.begin(), ordered.end(), [](const PatrolPoint& first, const PatrolPoint& second)
        {
            if (first.order != second.order)
            {
                return first.order < second.order;
            }
            if (first.row != second.row)
            {
                return first.row < second.row;
            }

            return first.column < second.column;
        });

        PatrolRoutes built;
        for (const PatrolPoint& point : ordered)
        {
            PatrolRoute* route = nullptr;
            for (PatrolRoute& candidate : built.routes)
            {
                if (candidate.id == point.routeId)
                {
                    route = &candidate;
                    break;
                }
            }

            if (route == nullptr)
            {
                built.routes.push_back({point.routeId, {}});
                route = &built.routes.back();
            }

            route->points.push_back(grid.ToWorld(point.column, point.row));
        }

        return built;
    }

    const PatrolRoutes& PatrolRoutes::Current()
    {
        return current;
    }

    void PatrolRoutes::SetCurrent(PatrolRoutes routes)
    {
        current = std::move(routes);
    }

    bool PatrolRoutes::IsEmpty() const
    {
        return routes.empty();
    }

    std::size_t PatrolRoutes::GetCount() const
    {
        return routes.size();
    }

    const PatrolRoute* PatrolRoutes::Find(const std::string& id) const
    {
        for (const PatrolRoute& route : routes)
        {
            if (route.id == id)
            {
                return &route;
            }
        }

        return nullptr;
    }

    const PatrolRoute* PatrolRoutes::Nearest(const Vector2Df& position, float maxDistance) const
    {
        const PatrolRoute* best = nullptr;
        float bestDistance = maxDistance * maxDistance;

        for (const PatrolRoute& route : routes)
        {
            for (const Vector2Df& point : route.points)
            {
                float distance = (point - position).GetLengthSquared();
                if (distance <= bestDistance)
                {
                    bestDistance = distance;
                    best = &route;
                }
            }
        }

        return best;
    }
}
