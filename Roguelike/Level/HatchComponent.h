#pragma once

#include <functional>
#include <string>
#include <EventList.h>
#include <InputSystem.h>
#include "Fixtures.h"
#include "InteractableComponent.h"

namespace XYZEngine
{
    class AudioComponent;
    class ColliderComponent;
    class SpriteRendererComponent;
}

namespace RoguelikeGame
{
    class HatchComponent : public InteractableComponent
    {
    public:
        HatchComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetHatchId(const std::string& newHatchId);
        const std::string& GetHatchId() const;
        void SetAudio(XYZEngine::AudioComponent* newAudio);
        void SetSprite(XYZEngine::SpriteRendererComponent* newSprite);
        void SetReachCollider(XYZEngine::ColliderComponent* reach);

        void Open();
        HatchState GetState() const;
        bool IsOpen() const;
        int GetFrame() const;

        std::string GetPrompt(XYZEngine::GameObject* actor) const override;
        bool IsAvailable() const override;
        bool Interact(XYZEngine::GameObject* actor) override;
        XYZEngine::InputAction GetAction() const override;

        XYZEngine::SubscriptionId SubscribeFled(std::function<void()> onFled);

    private:
        XYZEngine::AudioComponent* audio = nullptr;
        XYZEngine::SpriteRendererComponent* sprite = nullptr;

        std::string hatchId;
        HatchState state = HatchState::Shut;
        float since = 0.f;
        int frame = -1;
        bool hasFled = false;

        XYZEngine::EventList<> fledEvent;

        void ShowFrame(int wanted);
    };
}
