#include "BloodPool.h"
#include "GameSettings.h"
#include "Fx.h"
#include <GameWorld.h>

namespace RoguelikeGame
{
    void BloodPool::Spawn(const XYZEngine::Vector2Df& position, float angle)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(BLOOD_POOL_OBJECT_NAME);
        gameObject->SetRenderLayer(BLOOD_RENDER_LAYER);

        auto renderer = Fx::AddSprite(gameObject, BLOOD_POOL_TEXTURE, FX_BLOOD_POOL);
        if (renderer == nullptr)
        {
            XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
            return;
        }
        renderer->SetVisible(false);

        auto transform = gameObject->GetTransform();
        transform->SetWorldPosition(position);
        transform->SetWorldRotation(angle);

        auto animation = Fx::AddAnimation(gameObject, BLOOD_POOL_TEXTURE, FX_BLOOD_POOL);
        animation->SetStartDelay(BLOOD_POOL_DELAY);
        animation->SetEndBehaviour(XYZEngine::SpriteAnimationEnd::HoldLastFrame);
        animation->Play();
    }
}
