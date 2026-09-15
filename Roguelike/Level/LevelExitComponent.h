#pragma once

#include <functional>
#include <Component.h>
#include <EventList.h>

namespace XYZEngine
{
    class ColliderComponent;
    struct Trigger;
}

namespace RoguelikeGame
{
    class LevelExitComponent : public XYZEngine::Component
    {
    public:
        LevelExitComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        bool IsUsed() const;
        bool IsLocked() const;
        void SetLocked(bool newIsLocked);
        void Use();

        XYZEngine::SubscriptionId SubscribeEntered(std::function<void()> onEntered);
        XYZEngine::SubscriptionId SubscribeBlocked(std::function<void()> onBlocked);

    private:
        XYZEngine::ColliderComponent* collider = nullptr;
        bool isUsed = false;
        bool isLocked = false;
        bool isPlayerInside = false;

        XYZEngine::EventList<> enteredEvent;
        XYZEngine::EventList<> blockedEvent;

        bool IsPlayerTrigger(const XYZEngine::Trigger& trigger) const;
        void OnTriggerEnter(const XYZEngine::Trigger& trigger);
        void OnTriggerExit(const XYZEngine::Trigger& trigger);
        void TryEnter();
    };
}
