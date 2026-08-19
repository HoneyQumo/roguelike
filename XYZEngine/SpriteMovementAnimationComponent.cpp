#include "pch.h"
#include "SpriteMovementAnimationComponent.h"
#include "ResourceSystem.h"
#include <iostream>

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
			Play(idleAnimation);
		}
		else
		{
			Play(walkAnimation);
		}

		if (currentAnimation == nullptr || currentAnimation->frames.empty())
		{
			return;
		}

		frameTimer += deltaTime;
		while (frameTimer >= currentAnimation->secondsPerFrame)
		{
			frameTimer -= currentAnimation->secondsPerFrame;
			currentFrame = (currentFrame + 1) % (int)currentAnimation->frames.size();
		}

		renderer->SetTexture(*currentAnimation->frames[currentFrame]);
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

	void SpriteMovementAnimationComponent::Fill(Animation& animation, const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
	{
		animation.frames.clear();

		int totalFrames = ResourceSystem::Instance()->GetTextureMapElementsCount(textureMapName);
		if (firstFrameIndex < 0 || framesCount <= 0 || firstFrameIndex + framesCount > totalFrames)
		{
			std::cout << "Wrong animation frames range for texture map: " << textureMapName << std::endl;
			return;
		}

		if (framesPerSecond <= 0.f)
		{
			std::cout << "Framerate must be positive." << std::endl;
			return;
		}

		for (int i = firstFrameIndex; i < firstFrameIndex + framesCount; i++)
		{
			animation.frames.push_back(ResourceSystem::Instance()->GetTextureMapElementShared(textureMapName, i));
		}

		animation.secondsPerFrame = 1.f / framesPerSecond;
	}

	void SpriteMovementAnimationComponent::Play(Animation& animation)
	{
		if (currentAnimation == &animation)
		{
			return;
		}

		currentAnimation = &animation;
		frameTimer = 0.f;
		currentFrame = 0;
	}
}
