#pragma once

#include <functional>
#include <Component.h>
#include <Cooldown.h>
#include <EventList.h>

namespace RoguelikeGame
{
    class FuseComponent : public XYZEngine::Component
    {
    public:
        FuseComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void Light(float seconds);
        bool IsLit() const;
        bool HasBurnedOut() const;
        float GetLeft() const;

        XYZEngine::SubscriptionId SubscribeBurnedOut(std::function<void()> onBurnedOut);

    private:
        XYZEngine::Cooldown fuse;
        bool isLit = false;
        bool hasBurnedOut = false;

        XYZEngine::EventList<> burnedOutEvent;
    };
}
