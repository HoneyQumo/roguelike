#include "Navigator.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "PathService.h"
#include <DebugDraw.h>
#include <LoggerRegistry.h>

using namespace XYZEngine;

namespace RoguelikeGame
{
    void Navigator::Reset()
    {
        route.Clear();
        repath.Stop();
        progress = Unblock(progress);
        hasLastPosition = false;
    }

    bool Navigator::IsOnRoute() const
    {
        return route.HasPoint();
    }

    bool Navigator::IsBlocked() const
    {
        return RoguelikeGame::IsBlocked(progress);
    }

    const RouteFollower& Navigator::GetRoute() const
    {
        return route;
    }

    void Navigator::TakeRoute(const Vector2Df& position, const Vector2Df& goal)
    {
        std::vector<Vector2Df> steps;
        PathService::Current().RouteTo(position, goal, steps);
        route.SetRoute(std::move(steps));
        repath.Start(ENEMY_REPATH_INTERVAL);
    }

    Vector2Df Navigator::Steer(const Vector2Df& position, const Vector2Df& goal, float speed, float deltaTime)
    {
        float moved = hasLastPosition ? (position - lastPosition).GetLength() : 0.f;
        lastPosition = position;
        hasLastPosition = true;

        bool wasBlocked = RoguelikeGame::IsBlocked(progress);
        progress = WatchProgress(progress, moved, speed * deltaTime, deltaTime,
            NAVIGATION_PROGRESS_WINDOW, NAVIGATION_BLOCKED_TIME, NAVIGATION_MIN_PROGRESS);

        bool isBlocked = RoguelikeGame::IsBlocked(progress);
        if (isBlocked && !wasBlocked)
        {
            route.Clear();

            if (DebugDraw::Instance()->IsEnabled())
            {
                LOG_WARN("Navigation is blocked at "
                    + std::to_string(static_cast<int>(position.x)) + ";" + std::to_string(static_cast<int>(position.y))
                    + ", goal " + std::to_string(static_cast<int>(goal.x)) + ";" + std::to_string(static_cast<int>(goal.y))
                    + ", obstacle " + std::to_string(LevelGrid::Current().HasObstacleBetween(position, goal)));
            }
        }

        repath.Tick(deltaTime);

        if (isBlocked)
        {
            Vector2Df spot = position;
            if (LevelGrid::Current().FindFreeSpot(position, spot)
                && (spot - position).GetLength() > ENEMY_ROUTE_ARRIVE_DISTANCE)
            {
                route.Clear();

                return spot - position;
            }
        }

        if (!isBlocked && !LevelGrid::Current().HasObstacleBetween(position, goal))
        {
            route.Clear();
            repath.Stop();

            return goal - position;
        }

        if (!route.HasPoint() || repath.IsReady())
        {
            TakeRoute(position, goal);
        }

        route.Advance(position, ENEMY_ROUTE_ARRIVE_DISTANCE);

        return route.HasPoint() ? route.GetPoint() - position : goal - position;
    }
}
