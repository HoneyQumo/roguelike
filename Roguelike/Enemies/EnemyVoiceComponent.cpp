#include "EnemyVoiceComponent.h"
#include "ChaseComponent.h"
#include "EnemyVoice.h"
#include "GameResources.h"
#include "GameSettings.h"
#include "SpeechDirector.h"
#include <GameObject.h>
#include <randomizer.h>

namespace RoguelikeGame
{
    EnemyVoiceComponent::EnemyVoiceComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void EnemyVoiceComponent::Start()
    {
        chase = gameObject->GetComponent<ChaseComponent>();
    }

    void EnemyVoiceComponent::Update(float deltaTime)
    {
        if (chase == nullptr)
        {
            return;
        }

        AwarenessState now = chase->GetAwarenessState();
        if (IsSpottedNow(seenBefore, now))
        {
            Speak();
        }

        seenBefore = now;
    }

    void EnemyVoiceComponent::Render()
    {
    }

    void EnemyVoiceComponent::SetSpeech(const char* newSpeech)
    {
        speech = newSpeech;
    }

    int EnemyVoiceComponent::GetSpokenCount() const
    {
        return spokenCount;
    }

    void EnemyVoiceComponent::Speak()
    {
        spokenCount++;

        if (speech == nullptr)
        {
            return;
        }

        // Реплика идёт из точки врага: по голосу слышно, откуда тревога.
        SpeechDirector::Current().SayFromSet(speech, gameObject, SoundPlace::InWorld);
    }
}
