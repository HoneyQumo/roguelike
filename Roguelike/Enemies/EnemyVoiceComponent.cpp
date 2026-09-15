#include "EnemyVoiceComponent.h"
#include "ChaseComponent.h"
#include "EnemyVoice.h"
#include "GameResources.h"
#include "GameSettings.h"
#include <AudioComponent.h>
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

    void EnemyVoiceComponent::SetVoice(const char* newVoice, int newLines)
    {
        voice = newVoice;
        lines = newLines;
    }

    void EnemyVoiceComponent::SetAudio(XYZEngine::AudioComponent* newAudio)
    {
        audio = newAudio;
    }

    int EnemyVoiceComponent::GetSpokenCount() const
    {
        return spokenCount;
    }

    void EnemyVoiceComponent::Speak()
    {
        spokenCount++;

        if (audio == nullptr || voice == nullptr || lines <= 0)
        {
            return;
        }

        const sf::SoundBuffer* line = GameResources::GetVoiceLine(voice, random<int>(1, lines));
        if (line == nullptr)
        {
            return;
        }

        audio->SetSound(line);
        audio->SetVolume(VOICE_VOLUME);
        audio->Play();
    }
}
