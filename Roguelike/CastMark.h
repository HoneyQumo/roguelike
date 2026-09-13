#pragma once

#include <Component.h>
#include <Vector.h>

namespace XYZEngine
{
    class GameObject;
    class SpriteRendererComponent;
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
        int GetFrame() const;

    private:
        XYZEngine::SpriteRendererComponent* renderer = nullptr;
        float lifeTime = 0.f;
        float timeLeft = 0.f;
        float frameTime = 0.f;
        int frame = 0;
        bool isFinished = false;

        void ShowFrame();
    };

    XYZEngine::GameObject* CreateCastMark(const XYZEngine::Vector2Df& position, float radius, float lifeTime);
}
