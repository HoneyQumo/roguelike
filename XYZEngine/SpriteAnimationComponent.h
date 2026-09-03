#pragma once

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Component.h"
#include "SpriteRendererComponent.h"

namespace XYZEngine
{
    enum class SpriteAnimationEnd
    {
        HoldLastFrame,
        Hide,
        Destroy
    };

    class SpriteAnimationComponent : public Component
    {
    public:
        SpriteAnimationComponent(GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetFrames(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetLooped(bool newIsLooped);
        void SetEndBehaviour(SpriteAnimationEnd newEndBehaviour);
        void SetStartDelay(float newStartDelay);

        void Play();
        void Stop();
        bool IsPlaying() const;

    private:
        SpriteRendererComponent* renderer = nullptr;

        std::vector<const sf::Texture*> frames;
        float secondsPerFrame = 0.1f;
        SpriteAnimationEnd endBehaviour = SpriteAnimationEnd::HoldLastFrame;

        bool isLooped = false;
        bool isPlaying = false;

        float startDelay = 0.f;
        float delayTimer = 0.f;
        float frameTimer = 0.f;
        int currentFrame = 0;

        void ShowFirstFrame();

        void Finish();
    };
}
