#include "pch.h"
#include "SpriteMovementAnimationComponent.h"
#include "ResourceSystem.h"
#include "LoggerRegistry.h"
#include <cassert>

namespace XYZEngine
{
	const float MIN_MOVEMENT = 0.5f;
	const float STILL_DELAY = 0.15f;

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

		Vector2Df position = transform->GetWorldPosition();
		Vector2Df offset = position - previousPosition;
		previousPosition = position;

		if (isDead)
		{
			AdvanceFrames(deltaTime);
			return;
		}

		if (currentAnimation == &hurtAnimation && !isFinished)
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

		if (stillTimer > STILL_DELAY)
		{
			Play(idleAnimation, true);
		}
		else
		{
			Play(walkAnimation, true);
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
	void SpriteMovementAnimationComponent::SetHurtAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
	{
		Fill(hurtAnimation, textureMapName, firstFrameIndex, framesCount, framesPerSecond);
	}
	void SpriteMovementAnimationComponent::SetDeathAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
	{
		Fill(deathAnimation, textureMapName, firstFrameIndex, framesCount, framesPerSecond);
	}

	void SpriteMovementAnimationComponent::PlayHurt()
	{
		if (isDead || hurtAnimation.frames.empty())
		{
			return;
		}

		currentAnimation = nullptr;
		Play(hurtAnimation, false);
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
		Play(deathAnimation, false);
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

	void SpriteMovementAnimationComponent::Play(Animation& animation, bool looped)
	{
		if (currentAnimation == &animation)
		{
			return;
		}

		currentAnimation = &animation;
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
			while (frameTimer >= currentAnimation->secondsPerFrame)
			{
				frameTimer -= currentAnimation->secondsPerFrame;
				currentFrame++;

				if (currentFrame >= (int)currentAnimation->frames.size())
				{
					if (isLooped)
					{
						currentFrame = 0;
					}
					else
					{
						currentFrame = (int)currentAnimation->frames.size() - 1;
						isFinished = true;
					}
				}
			}
		}

		renderer->SetTexture(*currentAnimation->frames[currentFrame]);
	}
}
