#pragma once

#include <functional>
#include <Component.h>
#include <EventList.h>
#include <Vector.h>
#include "Traps.h"

namespace XYZEngine
{
    class AudioComponent;
    class GameObject;
}

namespace RoguelikeGame
{
    class PropVisualComponent;
    class TreadComponent;

    class TrapComponent : public XYZEngine::Component
    {
    public:
        TrapComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetKind(TrapKind newKind);
        TrapKind GetKind() const;
        void SetAmount(float newAmount);
        void SetTread(TreadComponent* newTread);
        void SetVisual(PropVisualComponent* newVisual);
        void SetAudio(XYZEngine::AudioComponent* newAudio);

        bool IsSprung() const;
        void Spring(XYZEngine::GameObject* walker);

        XYZEngine::SubscriptionId SubscribeSprung(std::function<void(XYZEngine::GameObject*)> onSprung);

    private:
        PropVisualComponent* visual = nullptr;
        XYZEngine::AudioComponent* audio = nullptr;

        TrapKind kind = TrapKind::None;
        float amount = 0.f;
        bool isSprung = false;

        XYZEngine::EventList<XYZEngine::GameObject*> sprungEvent;

        void Hurt(XYZEngine::GameObject* walker);
        void Alarm();
    };
}
