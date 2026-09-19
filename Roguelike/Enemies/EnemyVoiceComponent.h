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
        void SetNoticeSpeech(const char* newNotice);

        int GetSpokenCount() const;
        void Speak();
        void Notice();

    private:
        ChaseComponent* chase = nullptr;

        const char* speech = nullptr;
        const char* notice = nullptr;
        int spokenCount = 0;
        AwarenessState seenBefore = AwarenessState::Calm;
    };
}
