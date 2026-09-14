#include "DestructibleComponent.h"
#include "HealthComponent.h"
#include <ColliderComponent.h>
#include <GameObject.h>
#include <TransformComponent.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    DestructibleComponent::DestructibleComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void DestructibleComponent::Start()
    {
        health = gameObject->GetComponent<HealthComponent>();
        collider = gameObject->GetComponent<XYZEngine::ColliderComponent>();

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

        LOG_INFO(gameObject->GetName() + " is broken");

        brokenEvent.Invoke(gameObject->GetTransform()->GetWorldPosition());
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
