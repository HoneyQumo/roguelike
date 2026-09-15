#include "SwitchComponent.h"
#include "Fixtures.h"
#include "GameSettings.h"
#include "HatchComponent.h"
#include <AudioComponent.h>
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>

namespace RoguelikeGame
{
    SwitchComponent::SwitchComponent(XYZEngine::GameObject* gameObject) : InteractableComponent(gameObject) {}

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

        LOG_INFO("Lever " + switchId + " is pulled");
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

        std::string name = std::string(LEVER_TEXTURE_PREFIX) + std::to_string(LeverFrame(isPulled));
        const sf::Texture* texture = XYZEngine::ResourceSystem::Instance()->GetTextureShared(name);
        if (texture != nullptr)
        {
            sprite->SetTexture(*texture);
        }
    }

    void LinkSwitches(const std::vector<SwitchComponent*>& switches, const std::vector<HatchComponent*>& hatches)
    {
        for (SwitchComponent* lever : switches)
        {
            if (lever == nullptr)
            {
                continue;
            }

            std::vector<HatchComponent*> wanted;
            for (HatchComponent* hatch : hatches)
            {
                if (hatch != nullptr && hatch->GetHatchId() == lever->GetSwitchId())
                {
                    wanted.push_back(hatch);
                }
            }

            if (wanted.empty())
            {
                LOG_WARN("Lever " + lever->GetSwitchId() + " opens nothing");
                continue;
            }

            lever->SubscribePulled([wanted]()
            {
                for (HatchComponent* hatch : wanted)
                {
                    hatch->Open();
                }
            });
        }
    }
}
