#include "CastMark.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <RectangleRendererComponent.h>
#include <algorithm>

namespace RoguelikeGame
{
    CastMarkComponent::CastMarkComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void CastMarkComponent::Start()
    {
        renderer = gameObject->GetComponent<XYZEngine::RectangleRendererComponent>();
    }

    void CastMarkComponent::Update(float deltaTime)
    {
        if (isFinished)
        {
            return;
        }

        timeLeft = std::max(0.f, timeLeft - deltaTime);

        if (renderer != nullptr)
        {
            sf::Color color = BOSS_CAST_MARK_COLOR;
            color.a = static_cast<sf::Uint8>(color.a * GetPart());
            renderer->SetColor(color);
        }

        if (timeLeft <= 0.f)
        {
            isFinished = true;
            XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
        }
    }

    void CastMarkComponent::Render()
    {
    }

    void CastMarkComponent::SetLifeTime(float newLifeTime)
    {
        lifeTime = std::max(0.f, newLifeTime);
        timeLeft = lifeTime;
    }

    float CastMarkComponent::GetPart() const
    {
        return lifeTime <= 0.f ? 0.f : timeLeft / lifeTime;
    }

    XYZEngine::GameObject* CreateCastMark(const XYZEngine::Vector2Df& position, float radius, float lifeTime)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(CAST_MARK_OBJECT_NAME);
        gameObject->SetRenderLayer(GROUND_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::RectangleRendererComponent>();
        renderer->SetSize(2.f * radius, 2.f * radius);
        renderer->SetColor(BOSS_CAST_MARK_COLOR);

        gameObject->AddComponent<CastMarkComponent>()->SetLifeTime(lifeTime);

        return gameObject;
    }
}
