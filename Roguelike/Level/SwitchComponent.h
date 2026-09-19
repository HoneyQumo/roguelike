#pragma once

#include <functional>
#include <string>
#include <vector>
#include <EventList.h>
#include "InteractableComponent.h"
#include "Openable.h"

namespace XYZEngine
{
    class AudioComponent;
    class ColliderComponent;
    class SpriteRendererComponent;
}

namespace RoguelikeGame
{
    class SwitchComponent : public InteractableComponent
    {
    public:
        SwitchComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetSwitchId(const std::string& newSwitchId);
        void SetHoldTime(float newHoldTime);
        const std::string& GetSwitchId() const;
        void SetAudio(XYZEngine::AudioComponent* newAudio);
        void SetSprite(XYZEngine::SpriteRendererComponent* newSprite);
        void SetReachCollider(XYZEngine::ColliderComponent* reach);

        bool IsPulled() const;
        void Pull();

        std::string GetPrompt(XYZEngine::GameObject* actor) const override;
        bool IsAvailable() const override;
        bool Interact(XYZEngine::GameObject* actor) override;

        float GetHoldTime() const override;
        void OnHold(float part, float deltaTime) override;
        void OnHoldBroken() override;

        XYZEngine::SubscriptionId SubscribePulled(std::function<void()> onPulled);

    protected:
        // Наследник меняет только вид: механика у рычага и плитки одна.
        std::string texturePrefix;

        void ShowFrame();

    private:
        XYZEngine::AudioComponent* audio = nullptr;
        XYZEngine::SpriteRendererComponent* sprite = nullptr;

        std::string switchId;
        float holdTime = 0.f;
        float sinceNoise = 0.f;
        bool isPulled = false;

        XYZEngine::EventList<> pulledEvent;
    };

    void LinkSwitches(const std::vector<SwitchComponent*>& switches, const std::vector<Openable>& targets);
}
