#include "TireMark.h"
#include "GameSettings.h"
#include "Prop.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    void TireMark::Spawn(const XYZEngine::Vector2Df& position, float angle)
    {
        const sf::Texture* streak = XYZEngine::ResourceSystem::Instance()->GetTextureShared(
            PropTextureName(TIRE_MARK_PROP, false));
        if (streak == nullptr)
        {
            return;
        }

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(TIRE_MARK_OBJECT_NAME);
        gameObject->SetRenderLayer(TIRE_MARK_RENDER_LAYER);

        auto sprite = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        sprite->SetTexture(*streak);
        sprite->SetPixelSize(static_cast<int>(TIRE_MARK_LENGTH), static_cast<int>(TIRE_MARK_WIDTH));
        sprite->SetPivot(0.5f, 0.5f);

        auto transform = gameObject->GetTransform();
        transform->SetWorldPosition(position);
        transform->SetWorldRotation(angle);
    }
}
