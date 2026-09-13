#pragma once

#include <functional>
#include <string>
#include <Component.h>
#include <Vector.h>

namespace XYZEngine
{
    class GameObject;
    class SpriteRendererComponent;
    class TransformComponent;
}

namespace RoguelikeGame
{
    class CastMarkComponent : public XYZEngine::Component
    {
    public:
        CastMarkComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetRadius(float newRadius);
        void SetFuseTime(float newFuseTime);
        void SetTargetName(const std::string& newTargetName);
        void SetOwnerName(const std::string& newOwnerName);
        void SetOnDetonate(std::function<void(const XYZEngine::Vector2Df&)> newOnDetonate);

        bool IsFlying() const;
        float GetFusePart() const;
        int GetFrame() const;

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        XYZEngine::SpriteRendererComponent* renderer = nullptr;

        std::string targetName;
        std::string ownerName;
        std::function<void(const XYZEngine::Vector2Df&)> onDetonate;

        float radius = 0.f;
        float fuseTime = 0.f;
        float fuseLeft = 0.f;
        float flightLeft = 0.f;
        float frameTime = 0.f;
        int frame = 0;
        bool isFlying = true;
        bool isFinished = false;

        XYZEngine::GameObject* FindAlive(const std::string& name) const;
        void Land();
        void Detonate();
        void Cancel();
        void ShowFrame();
    };

    XYZEngine::GameObject* CreateCastMark(const XYZEngine::Vector2Df& position, float radius, float fuseTime);
}
