#include "Fixture.h"
#include "FogVisibilityComponent.h"
#include "Fixtures.h"
#include "GameSettings.h"
#include "EscapeCarComponent.h"
#include "HatchComponent.h"
#include "PlateComponent.h"
#include "SwitchComponent.h"
#include "WorldSound.h"
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
        PlaceInWorld(audio);
        auto reach = AddReach(gameObject);

        auto lever = gameObject->AddComponent<SwitchComponent>();
        lever->SetSwitchId(switchId);
        lever->SetSprite(sprite);
        lever->SetAudio(audio);
        lever->SetReachCollider(reach);
        HideInFog(gameObject);

        return gameObject;
    }

    // Плитка лежит в полу, поэтому и слой у неё земляной: по ней ходят, а не мимо неё.
    XYZEngine::GameObject* CreatePlate(const std::string& plateId, const XYZEngine::Vector2Df& position)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Plate_" + plateId);
        gameObject->SetRenderLayer(GROUND_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto sprite = AddSprite(gameObject);
        auto audio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        PlaceInWorld(audio);

        auto pad = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        pad->SetSize(FIXTURE_SIZE, FIXTURE_SIZE);
        pad->SetTrigger(true);
        pad->SetCollisionLayer(ITEM_COLLISION_LAYER);

        auto plate = gameObject->AddComponent<PlateComponent>();
        plate->SetSwitchId(plateId);
        plate->SetSprite(sprite);
        plate->SetAudio(audio);
        plate->SetPad(pad);
        HideInFog(gameObject);

        return gameObject;
    }

    XYZEngine::GameObject* CreateHatch(const std::string& hatchId, const XYZEngine::Vector2Df& position)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Hatch_" + hatchId);
        gameObject->SetRenderLayer(GROUND_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto sprite = AddSprite(gameObject);
        auto audio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        PlaceInWorld(audio);
        auto reach = AddReach(gameObject);

        auto hatch = gameObject->AddComponent<HatchComponent>();
        hatch->SetHatchId(hatchId);
        hatch->SetSprite(sprite);
        hatch->SetAudio(audio);
        hatch->SetReachCollider(reach);
        HideInFog(gameObject);

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
        car->SetParkPlace(position);

        // Звук живёт на машине, а не на сцене: сцена умирает сразу и оборвала бы его.
        // Мотор и визг звучат вместе, поэтому компонентов два: в одном помещается один звук.
        auto engineAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        auto skidAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
        PlaceInWorld(engineAudio);
        PlaceInWorld(skidAudio);

        car->SetEngineAudio(engineAudio);
        car->SetSkidAudio(skidAudio);

        return gameObject;
    }
}
