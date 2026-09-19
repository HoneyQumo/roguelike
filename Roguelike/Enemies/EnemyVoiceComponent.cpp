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
        else if (IsAlertedNow(seenBefore, now))
        {
            Notice();
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

    void EnemyVoiceComponent::SetNoticeSpeech(const char* newNotice)
    {
        notice = newNotice;
    }

    // Оклик - на переходе «спокоен - насторожился». Он занял место знака «?»
    // над головой: экран больше не рассказывает того, чего персонаж не видел.
    void EnemyVoiceComponent::Notice()
    {
        if (notice == nullptr)
        {
            return;
        }

        SpeechDirector::Current().SayFromSet(notice, gameObject, SoundPlace::InWorld);
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
