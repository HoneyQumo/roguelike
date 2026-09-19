#include "TreadComponent.h"
#include "FactionComponent.h"
#include <ColliderComponent.h>
#include <GameObject.h>
#include <Trigger.h>

namespace RoguelikeGame
{
    TreadComponent::TreadComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void TreadComponent::Start()
    {
    }

    void TreadComponent::Update(float deltaTime)
    {
    }

    void TreadComponent::Render()
    {
    }

    void TreadComponent::SetPad(XYZEngine::ColliderComponent* newPad)
    {
        pad = newPad;
        if (pad != nullptr)
        {
            pad->SubscribeTriggerEnter([this](const XYZEngine::Trigger& trigger) { OnEnter(trigger); });
        }
    }

    XYZEngine::SubscriptionId TreadComponent::SubscribeStepped(std::function<void(XYZEngine::GameObject*)> onStepped)
    {
        return steppedEvent.Subscribe(std::move(onStepped));
    }

    void TreadComponent::OnEnter(const XYZEngine::Trigger& trigger)
    {
        XYZEngine::ColliderComponent* other = trigger.GetFirst() == pad ? trigger.GetSecond() : trigger.GetFirst();
        if (other == nullptr)
        {
            return;
        }

        XYZEngine::GameObject* walker = other->GetGameObject();
        if (walker == nullptr || walker->GetComponent<FactionComponent>() == nullptr)
        {
            return;
        }

        steppedEvent.Invoke(walker);
    }
}
