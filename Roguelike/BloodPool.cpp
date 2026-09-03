#include "BloodPool.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <SpriteAnimationComponent.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    void BloodPool::Spawn(const XYZEngine::Vector2Df& position, float angle)
    {
        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(BLOOD_POOL_TEXTURE, 0);
        if (texture == nullptr)
        {
            LOG_ERROR("Blood pool texture is not loaded");
            return;
        }

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("BloodPool");
        gameObject->SetRenderLayer(BLOOD_RENDER_LAYER);

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetWorldPosition(position);
        transform->SetWorldRotation(angle);

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(FX_BLOOD_POOL.width, FX_BLOOD_POOL.height);
        renderer->SetVisible(false);

        auto animation = gameObject->AddComponent<XYZEngine::SpriteAnimationComponent>();
        animation->SetFrames(BLOOD_POOL_TEXTURE, 0, FX_BLOOD_POOL.frames, FramesPerSecond(FX_BLOOD_POOL.millisecondsPerFrame));
        animation->SetStartDelay(BLOOD_POOL_DELAY);

        animation->SetEndBehaviour(XYZEngine::SpriteAnimationEnd::HoldLastFrame);
        animation->Play();
    }
}
