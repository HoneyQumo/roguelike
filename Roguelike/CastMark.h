#pragma once

#include <Component.h>
#include <Vector.h>

namespace XYZEngine
{
    class GameObject;
    class RectangleRendererComponent;
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

        void SetLifeTime(float newLifeTime);
        float GetPart() const;

    private:
        XYZEngine::RectangleRendererComponent* renderer = nullptr;
        float lifeTime = 0.f;
        float timeLeft = 0.f;
        bool isFinished = false;
    };

    XYZEngine::GameObject* CreateCastMark(const XYZEngine::Vector2Df& position, float radius, float lifeTime);
}
