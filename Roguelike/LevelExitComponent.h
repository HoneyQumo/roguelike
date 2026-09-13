#pragma once

#include <functional>
#include <Component.h>
#include <EventList.h>

namespace XYZEngine
{
    class ColliderComponent;
    class Trigger;
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

        XYZEngine::SubscriptionId SubscribeEntered(std::function<void()> onEntered);

    private:
        XYZEngine::ColliderComponent* collider = nullptr;
        bool isUsed = false;

        XYZEngine::EventList<> enteredEvent;

        void OnTriggerEnter(const XYZEngine::Trigger& trigger);
    };
}
