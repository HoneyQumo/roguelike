#include "DestructibleComponent.h"
#include "HealthComponent.h"
#include <ColliderComponent.h>
#include <GameObject.h>
#include <RectangleRendererComponent.h>
#include <TransformComponent.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    DestructibleComponent::DestructibleComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void DestructibleComponent::Start()
    {
        health = gameObject->GetComponent<HealthComponent>();
        collider = gameObject->GetComponent<XYZEngine::ColliderComponent>();
        renderer = gameObject->GetComponent<XYZEngine::RectangleRendererComponent>();

        if (health == nullptr)
        {
            LOG_ERROR("Destructible needs a health component on " + gameObject->GetName());
            gameObject->DestroyComponent(this);
            return;
        }

        health->SubscribeDeath([this](const DeathInfo&) { Break(); });
    }

    void DestructibleComponent::Update(float deltaTime)
    {
    }

    void DestructibleComponent::Render()
    {
    }

    void DestructibleComponent::Break()
    {
        if (isBroken)
        {
            return;
        }

        isBroken = true;

        if (collider != nullptr)
        {
            collider->SetTrigger(true);
        }

        if (renderer != nullptr)
        {
            renderer->SetColor(brokenColor);
        }

        LOG_INFO(gameObject->GetName() + " is broken");

        brokenEvent.Invoke(gameObject->GetTransform()->GetWorldPosition());
    }

    void DestructibleComponent::SetBrokenColor(const sf::Color& newBrokenColor)
    {
        brokenColor = newBrokenColor;
    }

    bool DestructibleComponent::IsBroken() const
    {
        return isBroken;
    }

    XYZEngine::SubscriptionId DestructibleComponent::SubscribeBroken(std::function<void(const XYZEngine::Vector2Df&)> onBroken)
    {
        return brokenEvent.Subscribe(std::move(onBroken));
    }
}
