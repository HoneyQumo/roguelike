#include "SwitchComponent.h"
#include "Fixtures.h"
#include "Fx.h"
#include "GameSettings.h"
#include "Noise.h"
#include <AudioComponent.h>
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    SwitchComponent::SwitchComponent(XYZEngine::GameObject* gameObject)
        : InteractableComponent(gameObject), texturePrefix(LEVER_TEXTURE_PREFIX)
    {
    }

    void SwitchComponent::Start()
    {
        ShowFrame();
    }

    void SwitchComponent::Update(float deltaTime)
    {
    }

    void SwitchComponent::Render()
    {
    }

    void SwitchComponent::SetSwitchId(const std::string& newSwitchId)
    {
        switchId = newSwitchId;
    }

    const std::string& SwitchComponent::GetSwitchId() const
    {
        return switchId;
    }

    void SwitchComponent::SetHoldTime(float newHoldTime)
    {
        holdTime = newHoldTime;
    }

    float SwitchComponent::GetHoldTime() const
    {
        return holdTime;
    }

    // Пока держат - гремит, и охрана идёт смотреть. В этом и цена.
    void SwitchComponent::OnHold(float part, float deltaTime)
    {
        sinceNoise += deltaTime;
        if (sinceNoise < HOLD_NOISE_INTERVAL)
        {
            return;
        }

        sinceNoise = 0.f;
        holdTicks++;

        Noise noise;
        noise.position = gameObject->GetTransform()->GetWorldPosition();
        noise.radius = HOLD_NOISE_RADIUS;
        noise.loudness = HOLD_NOISE_LOUDNESS;
        RaiseNoise(noise);

        // Искры на том же такте: видно, что механизм ходит туго, даже если
        // смотришь на дверь, а не на полоску.
        Fx::SpawnImpact(gameObject->GetTransform()->GetWorldPosition(), {0.f, 1.f});

        // Шум слышит охрана, звук - игрок. Без него действие идёт молча,
        // и единственный признак, что оно идёт, - полоска внизу экрана.
        if (audio != nullptr)
        {
            const sf::SoundBuffer* sound = XYZEngine::ResourceSystem::Instance()->GetSound(LEVER_SOUND);
            if (sound != nullptr)
            {
                audio->SetSound(sound);
                audio->SetVolume(HOLD_VOLUME);
                audio->Play();
            }
        }
    }

    int SwitchComponent::GetHoldTicks() const
    {
        return holdTicks;
    }

    void SwitchComponent::OnHoldBroken()
    {
        sinceNoise = 0.f;
    }

    void SwitchComponent::SetAudio(XYZEngine::AudioComponent* newAudio)
    {
        audio = newAudio;
    }

    void SwitchComponent::SetSprite(XYZEngine::SpriteRendererComponent* newSprite)
    {
        sprite = newSprite;
    }

    void SwitchComponent::SetReachCollider(XYZEngine::ColliderComponent* reach)
    {
        BindReach(reach);
    }

    bool SwitchComponent::IsPulled() const
    {
        return isPulled;
    }

    void SwitchComponent::Pull()
    {
        if (isPulled)
        {
            return;
        }

        isPulled = true;
        ShowFrame();

        if (audio != nullptr)
        {
            const sf::SoundBuffer* sound = XYZEngine::ResourceSystem::Instance()->GetSound(LEVER_SOUND);
            if (sound != nullptr)
            {
                audio->SetSound(sound);
                audio->SetVolume(LEVER_VOLUME);
                audio->Play();
            }
        }

        LOG_INFO("Switch " + switchId + " is pulled");
        pulledEvent.Invoke();
    }

    std::string SwitchComponent::GetPrompt(XYZEngine::GameObject* actor) const
    {
        return LEVER_PROMPT;
    }

    bool SwitchComponent::IsAvailable() const
    {
        return !isPulled;
    }

    bool SwitchComponent::Interact(XYZEngine::GameObject* actor)
    {
        if (isPulled)
        {
            return false;
        }

        Pull();

        return true;
    }

    XYZEngine::SubscriptionId SwitchComponent::SubscribePulled(std::function<void()> onPulled)
    {
        return pulledEvent.Subscribe(std::move(onPulled));
    }

    void SwitchComponent::ShowFrame()
    {
        if (sprite == nullptr)
        {
            return;
        }

        std::string name = texturePrefix + std::to_string(LeverFrame(isPulled));
        const sf::Texture* texture = XYZEngine::ResourceSystem::Instance()->GetTextureShared(name);
        if (texture != nullptr)
        {
            sprite->SetTexture(*texture);
        }
    }

    void LinkSwitches(const std::vector<SwitchComponent*>& switches, const std::vector<Openable>& targets)
    {
        for (SwitchComponent* lever : switches)
        {
            if (lever == nullptr)
            {
                continue;
            }

            std::vector<Openable> wanted;
            for (const Openable& target : targets)
            {
                if (target.open != nullptr && target.id == lever->GetSwitchId())
                {
                    wanted.push_back(target);
                }
            }

            if (wanted.empty())
            {
                LOG_WARN("Lever " + lever->GetSwitchId() + " opens nothing");
                continue;
            }

            lever->SubscribePulled([wanted]()
            {
                for (const Openable& target : wanted)
                {
                    target.open();
                }
            });
        }
    }
}
