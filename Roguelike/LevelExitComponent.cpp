#include "LevelExitComponent.h"
#include "FactionComponent.h"
#include <ColliderComponent.h>
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <Trigger.h>

namespace RoguelikeGame
{
    LevelExitComponent::LevelExitComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void LevelExitComponent::Start()
    {
        collider = gameObject->GetComponent<XYZEngine::ColliderComponent>();
        if (collider == nullptr)
        {
            LOG_ERROR("Level exit needs a collider on " + gameObject->GetName());
            gameObject->DestroyComponent(this);
            return;
        }

        collider->SubscribeTriggerEnter([this](const XYZEngine::Trigger& trigger) { OnTriggerEnter(trigger); });
    }

    void LevelExitComponent::Update(float deltaTime)
    {
    }

    void LevelExitComponent::Render()
    {
    }

    bool LevelExitComponent::IsUsed() const
    {
        return isUsed;
    }

    void LevelExitComponent::OnTriggerEnter(const XYZEngine::Trigger& trigger)
    {
        if (isUsed)
        {
            return;
        }

        XYZEngine::ColliderComponent* other = trigger.GetFirst() == collider ? trigger.GetSecond() : trigger.GetFirst();
        if (other == nullptr || GetFactionOf(other->GetGameObject()) != Faction::Player)
        {
            return;
        }

        isUsed = true;
        LOG_INFO("Player reached the level exit");

        enteredEvent.Invoke();
    }

    XYZEngine::SubscriptionId LevelExitComponent::SubscribeEntered(std::function<void()> onEntered)
    {
        return enteredEvent.Subscribe(std::move(onEntered));
    }
}
