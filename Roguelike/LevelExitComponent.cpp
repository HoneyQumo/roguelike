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
        collider->SubscribeTriggerExit([this](const XYZEngine::Trigger& trigger) { OnTriggerExit(trigger); });
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

    bool LevelExitComponent::IsLocked() const
    {
        return isLocked;
    }

    void LevelExitComponent::SetLocked(bool newIsLocked)
    {
        isLocked = newIsLocked;

        if (!isLocked && isPlayerInside)
        {
            TryEnter();
        }
    }

    bool LevelExitComponent::IsPlayerTrigger(const XYZEngine::Trigger& trigger) const
    {
        XYZEngine::ColliderComponent* other = trigger.GetFirst() == collider ? trigger.GetSecond() : trigger.GetFirst();

        return other != nullptr && GetFactionOf(other->GetGameObject()) == Faction::Player;
    }

    void LevelExitComponent::OnTriggerEnter(const XYZEngine::Trigger& trigger)
    {
        if (!IsPlayerTrigger(trigger))
        {
            return;
        }

        isPlayerInside = true;
        TryEnter();
    }

    void LevelExitComponent::OnTriggerExit(const XYZEngine::Trigger& trigger)
    {
        if (!IsPlayerTrigger(trigger))
        {
            return;
        }

        isPlayerInside = false;
    }

    void LevelExitComponent::TryEnter()
    {
        if (isUsed)
        {
            return;
        }

        if (isLocked)
        {
            LOG_INFO("Level exit is locked while the boss is alive");
            blockedEvent.Invoke();
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

    XYZEngine::SubscriptionId LevelExitComponent::SubscribeBlocked(std::function<void()> onBlocked)
    {
        return blockedEvent.Subscribe(std::move(onBlocked));
    }
}
