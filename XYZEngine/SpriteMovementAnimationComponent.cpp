#include "pch.h"
#include "SpriteMovementAnimationComponent.h"
#include "ResourceSystem.h"
#include <iostream>

namespace XYZEngine
{
	const float MIN_MOVEMENT = 0.01f;

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

		if (frames.empty())
		{
			return;
		}

		Vector2Df position = transform->GetWorldPosition();
		Vector2Df offset = position - previousPosition;
		previousPosition = position;

		// A blocked object keeps its direction but stops moving, so the walk cycle has to stop too.
		if (offset.GetLength() < MIN_MOVEMENT)
		{
			frameTimer = 0.f;
			currentFrame = 0;
			ShowFrame(currentFrame);
			return;
		}

		frameTimer += deltaTime;
		while (frameTimer >= secondsPerFrame)
		{
			frameTimer -= secondsPerFrame;
			currentFrame = (currentFrame + 1) % (int)frames.size();
		}

		ShowFrame(currentFrame);
	}
	void SpriteMovementAnimationComponent::Render()
	{

	}

	void SpriteMovementAnimationComponent::SetFrames(const std::string& textureMapName, int firstFrameIndex, int framesCount)
	{
		frames.clear();

		int totalFrames = ResourceSystem::Instance()->GetTextureMapElementsCount(textureMapName);
		if (firstFrameIndex < 0 || framesCount <= 0 || firstFrameIndex + framesCount > totalFrames)
		{
			std::cout << "Wrong animation frames range for texture map: " << textureMapName << std::endl;
			return;
		}

		for (int i = firstFrameIndex; i < firstFrameIndex + framesCount; i++)
		{
			frames.push_back(ResourceSystem::Instance()->GetTextureMapElementShared(textureMapName, i));
		}
	}
	void SpriteMovementAnimationComponent::SetFramerate(float newFramesPerSecond)
	{
		if (newFramesPerSecond <= 0.f)
		{
			std::cout << "Framerate must be positive." << std::endl;
			return;
		}

		secondsPerFrame = 1.f / newFramesPerSecond;
	}

	void SpriteMovementAnimationComponent::ShowFrame(int frameIndex)
	{
		renderer->SetTexture(*frames[frameIndex]);
	}
}
