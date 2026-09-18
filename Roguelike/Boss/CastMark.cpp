#include "CastMark.h"
#include "BossSpriteAtlas.h"
#include "GameSettings.h"
#include "HealthComponent.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <TransformComponent.h>
#include <algorithm>
#include "EffectObject.h"

namespace RoguelikeGame
{
    CastMarkComponent::CastMarkComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
        flightLeft = BOSS_MARK_FLIGHT_TIME;
    }

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

        if (isFlying)
        {
            flightLeft -= deltaTime;
            if (flightLeft <= 0.f || FindAlive(ownerName) == nullptr)
            {
                Cancel();
                return;
            }

            XYZEngine::GameObject* target = FindAlive(targetName);
            if (target == nullptr)
            {
                Cancel();
                return;
            }

            XYZEngine::Vector2Df toTarget = target->GetTransform()->GetWorldPosition() - transform->GetWorldPosition();
            if (toTarget.GetLength() <= BOSS_MARK_TOUCH_RADIUS)
            {
                Land();
                return;
            }

            transform->SetWorldPosition(transform->GetWorldPosition()
                + toTarget.Normalized({1.f, 0.f}) * BOSS_MARK_SPEED * deltaTime);
            return;
        }

        frameTime += deltaTime;
        if (frameTime >= FX_PUPPETEER_MARK.secondsPerFrame)
        {
            frameTime -= FX_PUPPETEER_MARK.secondsPerFrame;
            frame = frame + 1 > FX_PUPPETEER_MARK_LOOP_LAST ? FX_PUPPETEER_MARK_LOOP_FIRST : frame + 1;
            ShowFrame();
        }

        fuseLeft = std::max(0.f, fuseLeft - deltaTime);
        if (fuseLeft <= 0.f)
        {
            Detonate();
        }
    }

    void CastMarkComponent::Render()
    {
    }

    XYZEngine::GameObject* CastMarkComponent::FindAlive(const std::string& name) const
    {
        if (name.empty())
        {
            return nullptr;
        }

        XYZEngine::GameObject* found = XYZEngine::GameWorld::Instance()->FindGameObject(name);
        if (found == nullptr)
        {
            return nullptr;
        }

        auto health = found->GetComponent<HealthComponent>();

        return health == nullptr || health->IsAlive() ? found : nullptr;
    }

    void CastMarkComponent::Land()
    {
        isFlying = false;
        fuseLeft = fuseTime;
        frame = 0;
        frameTime = 0.f;

        if (renderer != nullptr && renderer->GetSprite() != nullptr && renderer->GetSprite()->getTexture() != nullptr)
        {
            renderer->SetPixelSize(static_cast<int>(2.f * radius), static_cast<int>(2.f * radius));
        }

        ShowFrame();
    }

    void CastMarkComponent::Detonate()
    {
        isFinished = true;

        if (onDetonate != nullptr)
        {
            onDetonate(transform->GetWorldPosition());
        }

        XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
    }

    void CastMarkComponent::Cancel()
    {
        isFinished = true;
        XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
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

    void CastMarkComponent::SetRadius(float newRadius)
    {
        radius = std::max(0.f, newRadius);
    }

    void CastMarkComponent::SetFuseTime(float newFuseTime)
    {
        fuseTime = std::max(0.f, newFuseTime);
        fuseLeft = fuseTime;
    }

    void CastMarkComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
    }

    void CastMarkComponent::SetOwnerName(const std::string& newOwnerName)
    {
        ownerName = newOwnerName;
    }

    void CastMarkComponent::SetOnDetonate(std::function<void(const XYZEngine::Vector2Df&)> newOnDetonate)
    {
        onDetonate = std::move(newOnDetonate);
    }

    bool CastMarkComponent::IsFlying() const
    {
        return isFlying;
    }

    float CastMarkComponent::GetFusePart() const
    {
        return fuseTime <= 0.f ? 0.f : fuseLeft / fuseTime;
    }

    int CastMarkComponent::GetFrame() const
    {
        return frame;
    }

    XYZEngine::GameObject* CreateCastMark(const XYZEngine::Vector2Df& position, float radius, float fuseTime)
    {
        auto gameObject = CreateEffectObject(CAST_MARK_OBJECT_NAME, GROUND_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();

        const sf::Texture* texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(PUPPETEER_MARK_TEXTURE, 0);
        if (texture != nullptr)
        {
            renderer->SetTexture(*texture);
            renderer->SetPixelSize(BOSS_MARK_FLIGHT_SIZE, BOSS_MARK_FLIGHT_SIZE);
        }

        auto mark = gameObject->AddComponent<CastMarkComponent>();
        mark->SetRadius(radius);
        mark->SetFuseTime(fuseTime);

        return gameObject;
    }
}
