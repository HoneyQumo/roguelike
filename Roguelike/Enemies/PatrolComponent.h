#pragma once

#include <string>
#include <vector>
#include <Component.h>
#include <Cooldown.h>
#include <TransformComponent.h>
#include <MovementComponent.h>
#include "PatrolRules.h"
#include "RouteFollower.h"

namespace XYZEngine
{
    class AimRotationComponent;
}

namespace RoguelikeGame
{
    class ChaseComponent;

    class PatrolComponent : public XYZEngine::Component
    {
    public:
        PatrolComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetPoints(std::vector<XYZEngine::Vector2Df> newPoints);
        const std::vector<XYZEngine::Vector2Df>& GetPoints() const;

        bool IsWalking() const;
        std::size_t GetPointIndex() const;

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        XYZEngine::MovementComponent* movement = nullptr;
        XYZEngine::AimRotationComponent* aim = nullptr;
        ChaseComponent* chase = nullptr;

        std::vector<XYZEngine::Vector2Df> points;
        std::size_t index = 0u;
        bool wasEngaged = false;

        RouteFollower route;
        XYZEngine::Cooldown repath;

        void WalkTo(const XYZEngine::Vector2Df& goal, float deltaTime);
        void DrawRoute() const;
    };
}
