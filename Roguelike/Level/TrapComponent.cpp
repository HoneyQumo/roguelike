#include "TrapComponent.h"
#include "DamageInfo.h"
#include "FactionComponent.h"
#include "GameSettings.h"
#include "HealthComponent.h"
#include "Noise.h"
#include "PropVisualComponent.h"
#include "TreadComponent.h"
#include "WorldSound.h"
#include <AudioComponent.h>
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    TrapComponent::TrapComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void TrapComponent::Start()
    {
    }

    void TrapComponent::Update(float deltaTime)
    {
    }

    void TrapComponent::Render()
    {
    }

    void TrapComponent::SetKind(TrapKind newKind)
    {
        kind = newKind;
    }

    TrapKind TrapComponent::GetKind() const
    {
        return kind;
    }

    void TrapComponent::SetAmount(float newAmount)
    {
        amount = newAmount;
    }

    void TrapComponent::SetTread(TreadComponent* newTread)
    {
        if (newTread == nullptr)
        {
            return;
        }

        newTread->SubscribeStepped([this](XYZEngine::GameObject* walker) { Spring(walker); });
    }

    void TrapComponent::SetVisual(PropVisualComponent* newVisual)
    {
        visual = newVisual;
    }

    void TrapComponent::SetAudio(XYZEngine::AudioComponent* newAudio)
    {
        audio = newAudio;
    }

    bool TrapComponent::IsSprung() const
    {
        return isSprung;
    }

    void TrapComponent::Spring(XYZEngine::GameObject* walker)
    {
        if (isSprung || kind == TrapKind::None)
        {
            return;
        }

        isSprung = true;

        if (visual != nullptr)
        {
            visual->ShowSpent();
        }

        if (audio != nullptr)
        {
            const char* name = kind == TrapKind::Hurt ? TRAP_HURT_SOUND : TRAP_ALARM_SOUND;
            const sf::SoundBuffer* sound = XYZEngine::ResourceSystem::Instance()->GetSound(name);
            if (sound != nullptr)
            {
                audio->SetSound(sound);
                audio->SetVolume(TRAP_VOLUME);
                audio->Play();
            }
        }

        if (kind == TrapKind::Hurt)
        {
            Hurt(walker);
        }
        else
        {
            Alarm();
        }

        LOG_INFO("Trap " + gameObject->GetName() + " went off");
        sprungEvent.Invoke(walker);
    }

    XYZEngine::SubscriptionId TrapComponent::SubscribeSprung(std::function<void(XYZEngine::GameObject*)> onSprung)
    {
        return sprungEvent.Subscribe(std::move(onSprung));
    }

    void TrapComponent::Hurt(XYZEngine::GameObject* walker)
    {
        if (walker == nullptr)
        {
            return;
        }

        auto health = walker->GetComponent<HealthComponent>();
        if (health == nullptr)
        {
            return;
        }

        DamageSource source;
        source.kind = DamageKind::Melee;
        source.attackerName = gameObject->GetName();
        source.position = gameObject->GetTransform()->GetWorldPosition();

        health->TakeDamage(amount, source);

        // Вскрик и лязг слышно дальше самой ловушки: наступивший выдаёт себя.
        Noise noise;
        noise.position = source.position;
        noise.radius = TRAP_HURT_NOISE_RADIUS;
        RaiseNoise(noise);
    }

    void TrapComponent::Alarm()
    {
        Noise noise;
        noise.position = gameObject->GetTransform()->GetWorldPosition();
        noise.radius = amount;
        RaiseNoise(noise);
    }
}
