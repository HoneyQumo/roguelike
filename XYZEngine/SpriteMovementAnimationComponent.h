#pragma once

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Component.h"
#include "TransformComponent.h"
#include "SpriteRendererComponent.h"
#include "Vector.h"

namespace XYZEngine
{
    enum class MovementAnimation
    {
        None,
        Idle,
        Walk,
        Shoot,
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
        void SetIdleAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetShootAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetHurtAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetDeathAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);

        void PlayShoot();
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

        Animation walkAnimation;
        Animation idleAnimation;
        Animation shootAnimation;
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

        void Fill(Animation& animation, const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void Play(Animation& animation, MovementAnimation kind, bool looped);
        void AdvanceFrames(float deltaTime);
    };
}
