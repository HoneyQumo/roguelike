#pragma once

#include <Component.h>
#include "Awareness.h"

namespace XYZEngine
{
    class AudioComponent;
}

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

        void SetVoice(const char* newVoice, int newLines);
        void SetAudio(XYZEngine::AudioComponent* newAudio);

        int GetSpokenCount() const;
        void Speak();

    private:
        XYZEngine::AudioComponent* audio = nullptr;
        ChaseComponent* chase = nullptr;

        const char* voice = nullptr;
        int lines = 0;
        int spokenCount = 0;
        AwarenessState seenBefore = AwarenessState::Calm;
    };
}
