#pragma once

#include <string>
#include <vector>
#include <Component.h>
#include <Cooldown.h>
#include <TransformComponent.h>
#include <MovementComponent.h>
#include "LookAround.h"
#include "PatrolRules.h"
#include "Navigator.h"

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

        void SetPoints(std::vector<PatrolStop> newPoints);
        const std::vector<PatrolStop>& GetPoints() const;

        void SetLook(float newLookTime, float newHalfSweep);

        bool IsWalking() const;
        bool IsLooking() const;
        std::size_t GetPointIndex() const;

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        XYZEngine::MovementComponent* movement = nullptr;
        XYZEngine::AimRotationComponent* aim = nullptr;
        ChaseComponent* chase = nullptr;

        std::vector<PatrolStop> points;
        std::size_t index = 0u;
        bool wasEngaged = false;

        float lookTime = 0.f;
        float lookHalfSweep = 0.f;
        float lookElapsed = 0.f;
        bool isLooking = false;
        LookPlan lookPlan;

        Navigator navigator;

        // Возвращает сделанный шаг: по нему выставляется взгляд.
        XYZEngine::Vector2Df WalkTo(const XYZEngine::Vector2Df& goal, float deltaTime);
        void StartLook();
        void AimAtAngle(float degrees);
        void DrawRoute() const;
    };
}
