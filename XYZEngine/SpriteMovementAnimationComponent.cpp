#include "pch.h"
#include "SpriteMovementAnimationComponent.h"
#include "GameObject.h"
#include "ResourceSystem.h"
#include "LoggerRegistry.h"
#include <algorithm>
#include <cassert>

namespace XYZEngine
{
    constexpr float MIN_MOVEMENT = 0.5f;
    constexpr float DEFAULT_FRAME_SECONDS = 0.125f;
    constexpr float STILL_DELAY = 0.15f;

    SpriteMovementAnimationComponent::SpriteMovementAnimationComponent(GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
        previousPosition = transform->GetWorldPosition();
    }

    void SpriteMovementAnimationComponent::Start()
    {
        renderer = gameObject->GetComponent<SpriteRendererComponent>();
        movement = gameObject->GetComponent<MovementComponent>();
    }

    void SpriteMovementAnimationComponent::Update(float deltaTime)
    {
        if (renderer == nullptr)
        {
            return;
        }

        Vector2Df position = transform->GetWorldPosition();
        Vector2Df offset = position - previousPosition;
        previousPosition = position;

        if (isDead)
        {
            AdvanceFrames(deltaTime);
            return;
        }

        if (IsInterruptingAnimation() && !isFinished)
        {
            AdvanceFrames(deltaTime);
            return;
        }

        // Switching to idle is delayed, so a shaky contact with another object can't blink the animation.
        if (offset.GetLength() > MIN_MOVEMENT)
        {
            stillTimer = 0.f;
        }
        else
        {
            stillTimer += deltaTime;
        }

        bool isRunning = movement != nullptr && movement->IsRunning() && !runAnimation.IsEmpty();

        if (stillTimer > STILL_DELAY)
        {
            Play(idleAnimation, MovementAnimation::Idle, true);
        }
        else if (isRunning)
        {
            Play(runAnimation, MovementAnimation::Run, true);
        }
        else
        {
            Play(walkAnimation, MovementAnimation::Walk, true);
        }

        AdvanceFrames(deltaTime);
    }

    void SpriteMovementAnimationComponent::Render()
    {
    }

    void SpriteMovementAnimationComponent::SetWalkAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float secondsPerFrame)
    {
        walkAnimation.Load(textureMapName, firstFrameIndex, framesCount, secondsPerFrame);
    }

    void SpriteMovementAnimationComponent::SetIdleAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float secondsPerFrame)
    {
        idleAnimation.Load(textureMapName, firstFrameIndex, framesCount, secondsPerFrame);
    }

    void SpriteMovementAnimationComponent::SetRunAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float secondsPerFrame)
    {
        runAnimation.Load(textureMapName, firstFrameIndex, framesCount, secondsPerFrame);
    }

    void SpriteMovementAnimationComponent::SetShootAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float secondsPerFrame)
    {
        shootAnimation.Load(textureMapName, firstFrameIndex, framesCount, secondsPerFrame);
    }

    void SpriteMovementAnimationComponent::SetReloadAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float secondsPerFrame)
    {
        reloadAnimation.Load(textureMapName, firstFrameIndex, framesCount, secondsPerFrame);
    }

    void SpriteMovementAnimationComponent::SetMeleeAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float secondsPerFrame)
    {
        meleeAnimation.Load(textureMapName, firstFrameIndex, framesCount, secondsPerFrame);
    }

    void SpriteMovementAnimationComponent::SetHeavyAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, const float* frameSeconds,
                                                             const ChargedAnimationLoops& loops)
    {
        assert(frameSeconds != nullptr);

        heavyAnimation.Load(textureMapName, firstFrameIndex, framesCount, DEFAULT_FRAME_SECONDS);
        heavyAnimation.SetFrameSeconds(frameSeconds);
        heavyLoops = loops;
    }

    void SpriteMovementAnimationComponent::SetSwapAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, const float* frameSeconds)
    {
        assert(frameSeconds != nullptr);

        swapAnimation.Load(textureMapName, firstFrameIndex, framesCount, DEFAULT_FRAME_SECONDS);
        swapAnimation.SetFrameSeconds(frameSeconds);
    }

    void SpriteMovementAnimationComponent::SetHurtAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float secondsPerFrame)
    {
        hurtAnimation.Load(textureMapName, firstFrameIndex, framesCount, secondsPerFrame);
    }

    void SpriteMovementAnimationComponent::SetDeathAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float secondsPerFrame)
    {
        deathAnimation.Load(textureMapName, firstFrameIndex, framesCount, secondsPerFrame);
    }

    void SpriteMovementAnimationComponent::SetRollAnimations(const std::string& textureMapName, const int* firstFrameIndices, int directionsCount,
                                                             int framesCount, float secondsPerFrame)
    {
        assert(firstFrameIndices != nullptr);
        assert(directionsCount > 0 && directionsCount <= MAX_ROLL_DIRECTIONS);

        if (IsRollPlaying())
        {
            currentAnimation = nullptr;
            currentAnimationKind = MovementAnimation::None;
            isFinished = true;
        }

        rollDirectionsCount = std::min(std::max(directionsCount, 0), MAX_ROLL_DIRECTIONS);
        for (int direction = 0; direction < rollDirectionsCount; direction++)
        {
            rollAnimations[direction].Load(textureMapName, firstFrameIndices[direction], framesCount, secondsPerFrame);
        }
    }

    void SpriteMovementAnimationComponent::PlayShoot()
    {
        if (isDead || shootAnimation.IsEmpty() || IsRollPlaying())
        {
            return;
        }

        if ((currentAnimation == &hurtAnimation || currentAnimation == &reloadAnimation) && !isFinished)
        {
            return;
        }

        currentAnimation = nullptr;
        Play(shootAnimation, MovementAnimation::Shoot, false);
    }

    void SpriteMovementAnimationComponent::PlayReload()
    {
        if (isDead || reloadAnimation.IsEmpty() || IsRollPlaying())
        {
            return;
        }

        currentAnimation = nullptr;
        Play(reloadAnimation, MovementAnimation::Reload, false);
    }

    void SpriteMovementAnimationComponent::StopReload()
    {
        if (currentAnimation != &reloadAnimation)
        {
            return;
        }

        currentAnimation = nullptr;
        currentAnimationKind = MovementAnimation::None;
        isFinished = true;
    }

    void SpriteMovementAnimationComponent::PlayMelee()
    {
        if (isDead || meleeAnimation.IsEmpty() || IsRollPlaying())
        {
            return;
        }

        isHeavyHolding = false;
        currentAnimation = nullptr;
        Play(meleeAnimation, MovementAnimation::Melee, false);
    }

    void SpriteMovementAnimationComponent::PlayHeavy()
    {
        if (isDead || heavyAnimation.IsEmpty() || IsRollPlaying())
        {
            return;
        }

        isHeavyHolding = true;
        isHeavyCharged = false;
        currentAnimation = nullptr;
        Play(heavyAnimation, MovementAnimation::Heavy, false);
    }

    void SpriteMovementAnimationComponent::ReleaseHeavy()
    {
        isHeavyHolding = false;
    }

    void SpriteMovementAnimationComponent::SetHeavyCharged(bool newIsHeavyCharged)
    {
        isHeavyCharged = newIsHeavyCharged;
    }

    bool SpriteMovementAnimationComponent::IsHeavyHolding() const
    {
        return isHeavyHolding;
    }

    void SpriteMovementAnimationComponent::PlaySwap()
    {
        if (isDead || swapAnimation.IsEmpty() || IsRollPlaying())
        {
            return;
        }

        isHeavyHolding = false;
        currentAnimation = nullptr;
        Play(swapAnimation, MovementAnimation::Swap, false);
    }

    void SpriteMovementAnimationComponent::PlayHurt()
    {
        if (isDead || hurtAnimation.IsEmpty() || IsRollPlaying())
        {
            return;
        }

        if (currentAnimation == &reloadAnimation && !isFinished)
        {
            return;
        }

        currentAnimation = nullptr;
        Play(hurtAnimation, MovementAnimation::Hurt, false);
    }

    void SpriteMovementAnimationComponent::PlayDeath()
    {
        if (deathAnimation.IsEmpty())
        {
            LOG_WARN("No death animation on " + gameObject->GetName());
            return;
        }

        isDead = true;
        currentAnimation = nullptr;
        Play(deathAnimation, MovementAnimation::Death, false);
    }

    void SpriteMovementAnimationComponent::PlayRoll(int direction)
    {
        if (isDead || direction < 0 || direction >= rollDirectionsCount || rollAnimations[direction].IsEmpty())
        {
            return;
        }

        isHeavyHolding = false;
        currentAnimation = nullptr;
        Play(rollAnimations[direction], MovementAnimation::Roll, false);
    }

    MovementAnimation SpriteMovementAnimationComponent::GetCurrentAnimation() const
    {
        return currentAnimationKind;
    }

    int SpriteMovementAnimationComponent::GetCurrentFrame() const
    {
        return currentFrame;
    }

    bool SpriteMovementAnimationComponent::IsFinished() const
    {
        return isFinished;
    }

    int SpriteMovementAnimationComponent::GetRollDirectionsCount() const
    {
        return rollDirectionsCount;
    }

    bool SpriteMovementAnimationComponent::IsInterruptingAnimation() const
    {
        return currentAnimation == &hurtAnimation || currentAnimation == &shootAnimation || currentAnimation == &reloadAnimation
            || currentAnimation == &meleeAnimation || currentAnimation == &heavyAnimation || currentAnimation == &swapAnimation
            || currentAnimationKind == MovementAnimation::Roll;
    }

    bool SpriteMovementAnimationComponent::IsRollPlaying() const
    {
        return currentAnimationKind == MovementAnimation::Roll && !isFinished;
    }

    float SpriteMovementAnimationComponent::GetFrameSeconds(int frame) const
    {
        return currentAnimation == nullptr ? DEFAULT_FRAME_SECONDS : currentAnimation->GetFrameSeconds(frame);
    }

    bool SpriteMovementAnimationComponent::AdvanceHeavyFrame()
    {
        bool isInChargeLoop = currentFrame >= heavyLoops.chargeFirstFrame && currentFrame <= heavyLoops.chargeLastFrame;
        bool isInChargedLoop = currentFrame >= heavyLoops.chargedFirstFrame && currentFrame <= heavyLoops.chargedLastFrame;

        if (!isInChargeLoop && !isInChargedLoop)
        {
            return false;
        }

        if (!isHeavyHolding)
        {
            currentFrame = heavyLoops.releaseFrame;
            return true;
        }

        if (isInChargeLoop)
        {
            if (currentFrame < heavyLoops.chargeLastFrame)
            {
                currentFrame++;
            }
            else
            {
                currentFrame = isHeavyCharged ? heavyLoops.chargedFirstFrame : heavyLoops.chargeFirstFrame;
            }

            return true;
        }

        currentFrame = currentFrame < heavyLoops.chargedLastFrame ? currentFrame + 1 : heavyLoops.chargedFirstFrame;
        return true;
    }

    void SpriteMovementAnimationComponent::Play(AnimationClip& animation, MovementAnimation kind, bool looped)
    {
        if (currentAnimation == &animation)
        {
            return;
        }

        currentAnimation = &animation;
        currentAnimationKind = kind;
        isLooped = looped;
        isFinished = false;
        frameTimer = 0.f;
        currentFrame = 0;
    }

    void SpriteMovementAnimationComponent::AdvanceFrames(float deltaTime)
    {
        if (currentAnimation == nullptr || currentAnimation->IsEmpty())
        {
            return;
        }

        if (!isFinished)
        {
            frameTimer += deltaTime;
            while (frameTimer >= GetFrameSeconds(currentFrame))
            {
                frameTimer -= GetFrameSeconds(currentFrame);

                if (currentAnimation == &heavyAnimation && AdvanceHeavyFrame())
                {
                    continue;
                }

                currentFrame++;

                if (currentFrame >= currentAnimation->GetFramesCount())
                {
                    if (isLooped)
                    {
                        currentFrame = 0;
                    }
                    else
                    {
                        currentFrame = currentAnimation->GetFramesCount() - 1;
                        isFinished = true;
                    }
                }
            }
        }

        renderer->SetTexture(*currentAnimation->GetFrame(currentFrame));
    }
}
