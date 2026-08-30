#pragma once

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Component.h"
#include "TransformComponent.h"
#include "SpriteRendererComponent.h"
#include "MovementComponent.h"
#include "Vector.h"

namespace XYZEngine
{
    enum class MovementAnimation
    {
        None,
        Idle,
        Walk,
        Run,
        Shoot,
        Reload,
        Hurt,
        Death
    };

    class SpriteMovementAnimationComponent : public Component
    {
    public:
        SpriteMovementAnimationComponent(GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetWalkAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetRunAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetIdleAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetShootAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetReloadAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetHurtAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetDeathAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);

        void PlayShoot();
        void PlayReload();
        void StopReload();
        void PlayHurt();
        void PlayDeath();

        MovementAnimation GetCurrentAnimation() const;
        int GetCurrentFrame() const;

    private:
        struct Animation
        {
            std::vector<const sf::Texture*> frames;
            float secondsPerFrame = 0.125f;
        };

        TransformComponent* transform;
        SpriteRendererComponent* renderer = nullptr;
        MovementComponent* movement = nullptr;

        Animation walkAnimation;
        Animation runAnimation;
        Animation idleAnimation;
        Animation shootAnimation;
        Animation reloadAnimation;
        Animation hurtAnimation;
        Animation deathAnimation;
        Animation* currentAnimation = nullptr;
        MovementAnimation currentAnimationKind = MovementAnimation::None;

        bool isLooped = true;
        bool isFinished = false;
        bool isDead = false;

        float frameTimer = 0.f;
        int currentFrame = 0;

        Vector2Df previousPosition = {0.f, 0.f};
        float stillTimer = 0.f;

        bool IsInterruptingAnimation() const;
        void Fill(Animation& animation, const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void Play(Animation& animation, MovementAnimation kind, bool looped);
        void AdvanceFrames(float deltaTime);
    };
}
