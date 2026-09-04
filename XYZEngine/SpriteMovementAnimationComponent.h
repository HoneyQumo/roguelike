#pragma once

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "AnimationClip.h"
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
        Death,
        Roll
    };

    constexpr int MAX_ROLL_DIRECTIONS = 8;

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

        void Start() override;
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
        void SetRollAnimations(const std::string& textureMapName, const int* firstFrameIndices, int directionsCount, int framesCount,
                               float framesPerSecond);

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
        void PlayRoll(int direction);

        MovementAnimation GetCurrentAnimation() const;
        int GetCurrentFrame() const;
        bool IsFinished() const;
        int GetRollDirectionsCount() const;

    private:
        TransformComponent* transform = nullptr;
        SpriteRendererComponent* renderer = nullptr;
        MovementComponent* movement = nullptr;

        AnimationClip walkAnimation;
        AnimationClip runAnimation;
        AnimationClip idleAnimation;
        AnimationClip shootAnimation;
        AnimationClip reloadAnimation;
        AnimationClip meleeAnimation;
        AnimationClip heavyAnimation;
        AnimationClip swapAnimation;
        AnimationClip hurtAnimation;
        AnimationClip deathAnimation;
        AnimationClip rollAnimations[MAX_ROLL_DIRECTIONS];
        int rollDirectionsCount = 0;
        AnimationClip* currentAnimation = nullptr;
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
        bool IsRollPlaying() const;
        void Play(AnimationClip& animation, MovementAnimation kind, bool looped);
        void AdvanceFrames(float deltaTime);
        float GetFrameSeconds(int frame) const;
        bool AdvanceHeavyFrame();
    };
}
