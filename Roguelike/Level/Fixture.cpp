#include "Fixture.h"
#include "Fixtures.h"
#include "GameSettings.h"
#include "EscapeCarComponent.h"
#include "HatchComponent.h"
#include "SwitchComponent.h"
#include <AudioComponent.h>
#include <BoxColliderComponent.h>
#include <GameObject.h>
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    namespace
    {
        constexpr float FIXTURE_SIZE = 56.f;
        constexpr float FIXTURE_REACH_MARGIN = 28.f;

        XYZEngine::SpriteRendererComponent* AddSprite(XYZEngine::GameObject* gameObject)
        {
            auto sprite = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
            sprite->SetPixelSize(static_cast<int>(FIXTURE_SIZE), static_cast<int>(FIXTURE_SIZE));
            sprite->SetPivot(0.5f, 0.5f);

            return sprite;
        }

        XYZEngine::BoxColliderComponent* AddReach(XYZEngine::GameObject* gameObject)
        {
            auto reach = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
            reach->SetSize(FIXTURE_SIZE + FIXTURE_REACH_MARGIN, FIXTURE_SIZE + FIXTURE_REACH_MARGIN);
            reach->SetTrigger(true);
            reach->SetCollisionLayer(ITEM_COLLISION_LAYER);

            return reach;
        }
    }

    XYZEngine::GameObject* CreateLever(const std::string& switchId, const XYZEngine::Vector2Df& position)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Lever_" + switchId);
        gameObject->SetRenderLayer(ITEM_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto sprite = AddSprite(gameObject);
        auto audio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        auto reach = AddReach(gameObject);

        auto lever = gameObject->AddComponent<SwitchComponent>();
        lever->SetSwitchId(switchId);
        lever->SetSprite(sprite);
        lever->SetAudio(audio);
        lever->SetReachCollider(reach);

        return gameObject;
    }

    XYZEngine::GameObject* CreateHatch(const std::string& hatchId, const XYZEngine::Vector2Df& position)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Hatch_" + hatchId);
        gameObject->SetRenderLayer(GROUND_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto sprite = AddSprite(gameObject);
        auto audio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        auto reach = AddReach(gameObject);

        auto hatch = gameObject->AddComponent<HatchComponent>();
        hatch->SetHatchId(hatchId);
        hatch->SetSprite(sprite);
        hatch->SetAudio(audio);
        hatch->SetReachCollider(reach);

        return gameObject;
    }

    // Машина побега: крупнее прочих фикстур, берёт спрайт у фургона с моста.
    XYZEngine::GameObject* CreateEscapeCar(const std::string& carId, const XYZEngine::Vector2Df& position)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(ESCAPE_CAR_OBJECT_NAME);
        gameObject->SetRenderLayer(ITEM_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto sprite = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        sprite->SetPixelSize(static_cast<int>(ESCAPE_CAR_WIDTH), static_cast<int>(ESCAPE_CAR_HEIGHT));
        sprite->SetPivot(0.5f, 0.5f);

        const sf::Texture* body = XYZEngine::ResourceSystem::Instance()->GetTextureShared(ESCAPE_CAR_TEXTURE);
        if (body != nullptr)
        {
            sprite->SetTexture(*body);
        }

        auto reach = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        reach->SetSize(ESCAPE_CAR_WIDTH + FIXTURE_REACH_MARGIN, ESCAPE_CAR_HEIGHT + FIXTURE_REACH_MARGIN);
        reach->SetTrigger(true);
        reach->SetCollisionLayer(ITEM_COLLISION_LAYER);

        auto car = gameObject->AddComponent<EscapeCarComponent>();
        car->SetCarId(carId);
        car->SetReachCollider(reach);

        return gameObject;
    }
}
