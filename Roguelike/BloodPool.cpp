#include "BloodPool.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <LoggerRegistry.h>
#include <stdexcept>

namespace RoguelikeGame
{
    BloodPool::BloodPool()
    {
        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(BLOOD_POOL_TEXTURE, 0);
        if (texture == nullptr)
        {
            throw std::runtime_error("blood pool texture is not loaded");
        }

        gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("BloodPool");

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(FX_BLOOD_POOL.width, FX_BLOOD_POOL.height);
        renderer->SetVisible(false);

        animation = gameObject->AddComponent<XYZEngine::SpriteAnimationComponent>();
        animation->SetFrames(BLOOD_POOL_TEXTURE, 0, FX_BLOOD_POOL.frames, FramesPerSecond(FX_BLOOD_POOL.millisecondsPerFrame));
        animation->SetStartDelay(BLOOD_POOL_DELAY);
        animation->SetEndBehaviour(XYZEngine::SpriteAnimationEnd::HoldLastFrame);
    }

    void BloodPool::AttachTo(XYZEngine::GameObject* owner)
    {
        if (owner == nullptr)
        {
            return;
        }

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetParent(owner->GetComponent<XYZEngine::TransformComponent>());
        transform->SetLocalPosition(0.f, 0.f);
    }

    XYZEngine::SpriteAnimationComponent* BloodPool::GetAnimation()
    {
        return animation;
    }
}
