#pragma once

#include <vector>
#include <Cooldown.h>
#include <Vector.h>
#include "ProgressWatch.h"
#include "RouteFollower.h"

namespace RoguelikeGame
{
    class Navigator
    {
    public:
        XYZEngine::Vector2Df Steer(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& goal,
            float speed, float deltaTime);
        void Reset();

        bool IsOnRoute() const;
        bool IsBlocked() const;
        const RouteFollower& GetRoute() const;

    private:
        void TakeRoute(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& goal);

        RouteFollower route;
        XYZEngine::Cooldown repath;
        ProgressWatch progress;
        XYZEngine::Vector2Df lastPosition = {0.f, 0.f};
        bool hasLastPosition = false;
    };
}
