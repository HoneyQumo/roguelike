#pragma once

#include <vector>
#include <Vector.h>

namespace RoguelikeGame
{
    class RouteFollower
    {
    public:
        void SetRoute(std::vector<XYZEngine::Vector2Df> newRoute)
        {
            route = std::move(newRoute);
            index = 0u;
        }

        void Clear()
        {
            route.clear();
            index = 0u;
        }

        void Advance(const XYZEngine::Vector2Df& position, float arriveDistance)
        {
            while (index < route.size() && (route[index] - position).GetLength() <= arriveDistance)
            {
                index++;
            }
        }

        bool HasPoint() const
        {
            return index < route.size();
        }

        const XYZEngine::Vector2Df& GetPoint() const
        {
            return route[index];
        }

        std::size_t GetRemaining() const
        {
            return route.size() - index;
        }

        const std::vector<XYZEngine::Vector2Df>& GetRoute() const
        {
            return route;
        }

        std::size_t GetIndex() const
        {
            return index;
        }

    private:
        std::vector<XYZEngine::Vector2Df> route;
        std::size_t index = 0u;
    };
}
