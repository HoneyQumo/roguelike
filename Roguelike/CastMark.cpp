#include "CastMark.h"
#include "BossSpriteAtlas.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <algorithm>

namespace RoguelikeGame
{
    CastMarkComponent::CastMarkComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void CastMarkComponent::Start()
    {
        renderer = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();
    }

    void CastMarkComponent::Update(float deltaTime)
    {
        if (isFinished)
        {
            return;
        }

        timeLeft = std::max(0.f, timeLeft - deltaTime);
        frameTime += deltaTime;

        if (frameTime >= FX_PUPPETEER_MARK.secondsPerFrame)
        {
            frameTime -= FX_PUPPETEER_MARK.secondsPerFrame;
            frame = frame + 1 >= FX_PUPPETEER_MARK.frames ? FX_PUPPETEER_MARK_LOOP_FIRST : frame + 1;

            if (frame > FX_PUPPETEER_MARK_LOOP_LAST)
            {
                frame = FX_PUPPETEER_MARK_LOOP_FIRST;
            }

            ShowFrame();
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

    void CastMarkComponent::ShowFrame()
    {
        if (renderer == nullptr)
        {
            return;
        }

        const sf::Texture* texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(PUPPETEER_MARK_TEXTURE, frame);
        if (texture != nullptr)
        {
            renderer->SetTexture(*texture);
        }
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

    int CastMarkComponent::GetFrame() const
    {
        return frame;
    }

    XYZEngine::GameObject* CreateCastMark(const XYZEngine::Vector2Df& position, float radius, float lifeTime)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(CAST_MARK_OBJECT_NAME);
        gameObject->SetRenderLayer(GROUND_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();

        const sf::Texture* texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(PUPPETEER_MARK_TEXTURE, 0);
        if (texture != nullptr)
        {
            renderer->SetTexture(*texture);
            renderer->SetPixelSize(static_cast<int>(2.f * radius), static_cast<int>(2.f * radius));
        }

        gameObject->AddComponent<CastMarkComponent>()->SetLifeTime(lifeTime);

        return gameObject;
    }
}
