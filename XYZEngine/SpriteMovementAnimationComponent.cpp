#include "pch.h"
#include "SpriteMovementAnimationComponent.h"
#include "ResourceSystem.h"
#include "LoggerRegistry.h"
#include <algorithm>
#include <cassert>

namespace XYZEngine
{
    constexpr float MIN_MOVEMENT = 0.5f;
    constexpr float STILL_DELAY = 0.15f;

    SpriteMovementAnimationComponent::SpriteMovementAnimationComponent(GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetComponent<TransformComponent>();
        previousPosition = transform->GetWorldPosition();
    }

    void SpriteMovementAnimationComponent::Update(float deltaTime)
    {
        if (renderer == nullptr)
        {
            renderer = gameObject->GetComponent<SpriteRendererComponent>();
            if (renderer == nullptr)
            {
                return;
            }
        }

        if (movement == nullptr)
        {
            movement = gameObject->GetComponent<MovementComponent>();
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

        bool isRunning = movement != nullptr && movement->IsRunning() && !runAnimation.frames.empty();

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

    void SpriteMovementAnimationComponent::SetWalkAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
    {
        Fill(walkAnimation, textureMapName, firstFrameIndex, framesCount, framesPerSecond);
    }

    void SpriteMovementAnimationComponent::SetIdleAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
    {
        Fill(idleAnimation, textureMapName, firstFrameIndex, framesCount, framesPerSecond);
    }

    void SpriteMovementAnimationComponent::SetRunAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
    {
        Fill(runAnimation, textureMapName, firstFrameIndex, framesCount, framesPerSecond);
    }

    void SpriteMovementAnimationComponent::SetShootAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
    {
        Fill(shootAnimation, textureMapName, firstFrameIndex, framesCount, framesPerSecond);
    }

    void SpriteMovementAnimationComponent::SetReloadAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
    {
        Fill(reloadAnimation, textureMapName, firstFrameIndex, framesCount, framesPerSecond);
    }

    void SpriteMovementAnimationComponent::SetMeleeAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
    {
        Fill(meleeAnimation, textureMapName, firstFrameIndex, framesCount, framesPerSecond);
    }

    void SpriteMovementAnimationComponent::SetHeavyAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, const float* frameSeconds,
                                                             const ChargedAnimationLoops& loops)
    {
        assert(frameSeconds != nullptr);

        Fill(heavyAnimation, textureMapName, firstFrameIndex, framesCount, 1.f);
        heavyAnimation.frameSeconds = frameSeconds;
        heavyLoops = loops;
    }

    void SpriteMovementAnimationComponent::SetSwapAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, const float* frameSeconds)
    {
        assert(frameSeconds != nullptr);

        Fill(swapAnimation, textureMapName, firstFrameIndex, framesCount, 1.f);
        swapAnimation.frameSeconds = frameSeconds;
    }

    void SpriteMovementAnimationComponent::SetHurtAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
    {
        Fill(hurtAnimation, textureMapName, firstFrameIndex, framesCount, framesPerSecond);
    }

    void SpriteMovementAnimationComponent::SetDeathAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
    {
        Fill(deathAnimation, textureMapName, firstFrameIndex, framesCount, framesPerSecond);
    }

    void SpriteMovementAnimationComponent::PlayShoot()
    {
        if (isDead || shootAnimation.frames.empty())
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
        if (isDead || reloadAnimation.frames.empty())
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
        if (isDead || meleeAnimation.frames.empty())
        {
            return;
        }

        isHeavyHolding = false;
        currentAnimation = nullptr;
        Play(meleeAnimation, MovementAnimation::Melee, false);
    }

    void SpriteMovementAnimationComponent::PlayHeavy()
    {
        if (isDead || heavyAnimation.frames.empty())
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
        if (isDead || swapAnimation.frames.empty())
        {
            return;
        }

        isHeavyHolding = false;
        currentAnimation = nullptr;
        Play(swapAnimation, MovementAnimation::Swap, false);
    }

    void SpriteMovementAnimationComponent::PlayHurt()
    {
        if (isDead || hurtAnimation.frames.empty())
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
        if (deathAnimation.frames.empty())
        {
            LOG_WARN("No death animation on " + gameObject->GetName());
            return;
        }

        isDead = true;
        currentAnimation = nullptr;
        Play(deathAnimation, MovementAnimation::Death, false);
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

    bool SpriteMovementAnimationComponent::IsInterruptingAnimation() const
    {
        return currentAnimation == &hurtAnimation || currentAnimation == &shootAnimation || currentAnimation == &reloadAnimation
            || currentAnimation == &meleeAnimation || currentAnimation == &heavyAnimation || currentAnimation == &swapAnimation;
    }

    float SpriteMovementAnimationComponent::GetFrameSeconds(int frame) const
    {
        if (currentAnimation == nullptr)
        {
            return 0.125f;
        }

        if (currentAnimation->frameSeconds == nullptr)
        {
            return currentAnimation->secondsPerFrame;
        }

        return currentAnimation->frameSeconds[std::min(std::max(frame, 0), static_cast<int>(currentAnimation->frames.size()) - 1)];
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

    void SpriteMovementAnimationComponent::Fill(Animation& animation, const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
    {
        assert(framesCount > 0);
        assert(framesPerSecond > 0.f);

        animation.frames.clear();

        int totalFrames = ResourceSystem::Instance()->GetTextureMapElementsCount(textureMapName);
        if (firstFrameIndex < 0 || framesCount <= 0 || firstFrameIndex + framesCount > totalFrames)
        {
            LOG_ERROR("Wrong animation frames range for texture map: " + textureMapName);
            return;
        }

        if (framesPerSecond <= 0.f)
        {
            LOG_WARN("Framerate must be positive for texture map: " + textureMapName);
            return;
        }

        for (int i = firstFrameIndex; i < firstFrameIndex + framesCount; i++)
        {
            animation.frames.push_back(ResourceSystem::Instance()->GetTextureMapElementShared(textureMapName, i));
        }

        animation.secondsPerFrame = 1.f / framesPerSecond;
    }

    void SpriteMovementAnimationComponent::Play(Animation& animation, MovementAnimation kind, bool looped)
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
        if (currentAnimation == nullptr || currentAnimation->frames.empty())
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

                if (currentFrame >= static_cast<int>(currentAnimation->frames.size()))
                {
                    if (isLooped)
                    {
                        currentFrame = 0;
                    }
                    else
                    {
                        currentFrame = static_cast<int>(currentAnimation->frames.size()) - 1;
                        isFinished = true;
                    }
                }
            }
        }

        renderer->SetTexture(*currentAnimation->frames[currentFrame]);
    }
}
