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
        Melee,
        Heavy,
        Swap,
        Hurt,
        Death
    };

    struct ChargedAnimationLoops
    {
        int chargeFirstFrame = 0;
        int chargeLastFrame = 0;
        int chargedFirstFrame = 0;
        int chargedLastFrame = 0;
        int releaseFrame = 0;
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
        void SetMeleeAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetHeavyAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, const float* frameSeconds,
                               const ChargedAnimationLoops& loops);
        void SetSwapAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, const float* frameSeconds);
        void SetHurtAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void SetDeathAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);

        void PlayShoot();
        void PlayReload();
        void StopReload();
        void PlayMelee();
        void PlayHeavy();
        void ReleaseHeavy();
        void SetHeavyCharged(bool newIsHeavyCharged);
        bool IsHeavyHolding() const;
        void PlaySwap();
        void PlayHurt();
        void PlayDeath();

        MovementAnimation GetCurrentAnimation() const;
        int GetCurrentFrame() const;
        bool IsFinished() const;

    private:
        struct Animation
        {
            std::vector<const sf::Texture*> frames;
            float secondsPerFrame = 0.125f;
            const float* frameSeconds = nullptr;
        };

        TransformComponent* transform;
        SpriteRendererComponent* renderer = nullptr;
        MovementComponent* movement = nullptr;

        Animation walkAnimation;
        Animation runAnimation;
        Animation idleAnimation;
        Animation shootAnimation;
        Animation reloadAnimation;
        Animation meleeAnimation;
        Animation heavyAnimation;
        Animation swapAnimation;
        Animation hurtAnimation;
        Animation deathAnimation;
        Animation* currentAnimation = nullptr;
        MovementAnimation currentAnimationKind = MovementAnimation::None;

        bool isLooped = true;
        bool isFinished = false;
        bool isDead = false;

        ChargedAnimationLoops heavyLoops;
        bool isHeavyHolding = false;
        bool isHeavyCharged = false;

        float frameTimer = 0.f;
        int currentFrame = 0;

        Vector2Df previousPosition = {0.f, 0.f};
        float stillTimer = 0.f;

        bool IsInterruptingAnimation() const;
        void Fill(Animation& animation, const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
        void Play(Animation& animation, MovementAnimation kind, bool looped);
        void AdvanceFrames(float deltaTime);
        float GetFrameSeconds(int frame) const;
        bool AdvanceHeavyFrame();
    };
}
