#pragma once

#include <Component.h>
#include "Awareness.h"

namespace RoguelikeGame
{
    class ChaseComponent;

    class EnemyVoiceComponent : public XYZEngine::Component
    {
    public:
        EnemyVoiceComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetSpeech(const char* newSpeech);

        int GetSpokenCount() const;
        void Speak();

    private:
        ChaseComponent* chase = nullptr;

        const char* speech = nullptr;
        int spokenCount = 0;
        AwarenessState seenBefore = AwarenessState::Calm;
    };
}
