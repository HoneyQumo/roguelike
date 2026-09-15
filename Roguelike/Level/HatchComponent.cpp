#include "HatchComponent.h"
#include "GameSettings.h"
#include <AudioComponent.h>
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>

namespace RoguelikeGame
{
    namespace
    {
        std::string FrameName(int frame)
        {
            return std::string(HATCH_TEXTURE_PREFIX) + std::to_string(frame);
        }
    }

    HatchComponent::HatchComponent(XYZEngine::GameObject* gameObject) : InteractableComponent(gameObject) {}

    void HatchComponent::Start()
    {
        ShowFrame(HatchFrame(state, since, HATCH_OPEN_TIME));
    }

    void HatchComponent::Update(float deltaTime)
    {
        if (state != HatchState::Opening)
        {
            return;
        }

        since += deltaTime;
        state = NextHatchState(state, since, HATCH_OPEN_TIME);
        ShowFrame(HatchFrame(state, since, HATCH_OPEN_TIME));
    }

    void HatchComponent::Render()
    {
    }

    void HatchComponent::SetHatchId(const std::string& newHatchId)
    {
        hatchId = newHatchId;
    }

    const std::string& HatchComponent::GetHatchId() const
    {
        return hatchId;
    }

    void HatchComponent::SetAudio(XYZEngine::AudioComponent* newAudio)
    {
        audio = newAudio;
    }

    void HatchComponent::SetSprite(XYZEngine::SpriteRendererComponent* newSprite)
    {
        sprite = newSprite;
    }

    void HatchComponent::SetReachCollider(XYZEngine::ColliderComponent* reach)
    {
        BindReach(reach);
    }

    void HatchComponent::Open()
    {
        if (state != HatchState::Shut)
        {
            return;
        }

        state = HatchState::Opening;
        since = 0.f;

        if (audio != nullptr)
        {
            const sf::SoundBuffer* sound = XYZEngine::ResourceSystem::Instance()->GetSound(HATCH_SOUND);
            if (sound != nullptr)
            {
                audio->SetSound(sound);
                audio->SetVolume(HATCH_VOLUME);
                audio->Play();
            }
        }

        LOG_INFO("Hatch " + hatchId + " is opening");
    }

    HatchState HatchComponent::GetState() const
    {
        return state;
    }

    bool HatchComponent::IsOpen() const
    {
        return state == HatchState::Open;
    }

    int HatchComponent::GetFrame() const
    {
        return frame;
    }

    std::string HatchComponent::GetPrompt(XYZEngine::GameObject* actor) const
    {
        return HATCH_FLEE_PROMPT;
    }

    bool HatchComponent::IsAvailable() const
    {
        return state == HatchState::Open && !hasFled;
    }

    bool HatchComponent::Interact(XYZEngine::GameObject* actor)
    {
        if (!IsAvailable())
        {
            return false;
        }

        hasFled = true;
        LOG_INFO("Hatch " + hatchId + " is used to flee");
        fledEvent.Invoke();

        return true;
    }

    XYZEngine::InputAction HatchComponent::GetAction() const
    {
        return XYZEngine::InputAction::Flee;
    }

    XYZEngine::SubscriptionId HatchComponent::SubscribeFled(std::function<void()> onFled)
    {
        return fledEvent.Subscribe(std::move(onFled));
    }

    void HatchComponent::ShowFrame(int wanted)
    {
        if (sprite == nullptr || wanted == frame)
        {
            return;
        }

        const sf::Texture* texture = XYZEngine::ResourceSystem::Instance()->GetTextureShared(FrameName(wanted));
        if (texture == nullptr)
        {
            return;
        }

        sprite->SetTexture(*texture);
        frame = wanted;
    }
}
