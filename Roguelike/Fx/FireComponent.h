#pragma once

#include <functional>
#include <Component.h>
#include <Cooldown.h>
#include <EventList.h>
#include <Vector.h>

namespace XYZEngine
{
    class TransformComponent;
}

namespace RoguelikeGame
{
    /**
    *	Огонь: горит отмеренное время и жжёт всех, кто в него зашёл.
    *	Ничего не знает о том, что именно горит - машина, бочка или просто асфальт.
    */
    class FireComponent : public XYZEngine::Component
    {
    public:
        FireComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void Light(float seconds);
        void Throw(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to, float seconds);

        void SetRadius(float newRadius);
        void SetDamage(float newDamage);
        void SetBeatTime(float newBeatTime);

        bool IsBurning() const;
        bool IsFlying() const;
        float GetLeft() const;

        XYZEngine::SubscriptionId SubscribeBurnedOut(std::function<void()> onBurnedOut);

    private:
        XYZEngine::TransformComponent* transform = nullptr;

        XYZEngine::Vector2Df from = {0.f, 0.f};
        XYZEngine::Vector2Df to = {0.f, 0.f};

        float flightTime = 0.f;
        float inFlight = 0.f;

        float left = 0.f;
        float radius = 0.f;
        float damage = 0.f;
        float beatTime = 0.f;
        float sinceBeat = 0.f;

        XYZEngine::EventList<> burnedOutEvent;

        void Fly(float deltaTime);
        void Burn(float deltaTime);
        void Sting();
    };
}
