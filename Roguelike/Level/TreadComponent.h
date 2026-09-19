#pragma once

#include <functional>
#include <Component.h>
#include <EventList.h>

namespace XYZEngine
{
    class ColliderComponent;
    class GameObject;
    struct Trigger;
}

namespace RoguelikeGame
{
    /**
    *	«На меня наступили». Повод общий у нажимной плитки и у ловушки,
    *	а что дальше - дело подписчика: одна открывает дверь, другая бьёт по ногам.
    *
    *	Считаются только те, у кого есть сторона: пуля над плиткой пролетает зря.
    */
    class TreadComponent : public XYZEngine::Component
    {
    public:
        TreadComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetPad(XYZEngine::ColliderComponent* newPad);

        XYZEngine::SubscriptionId SubscribeStepped(std::function<void(XYZEngine::GameObject*)> onStepped);

    private:
        XYZEngine::ColliderComponent* pad = nullptr;

        XYZEngine::EventList<XYZEngine::GameObject*> steppedEvent;

        void OnEnter(const XYZEngine::Trigger& trigger);
    };
}
