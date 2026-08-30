#include "pch.h"
#include "SpriteAnimationComponent.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "ResourceSystem.h"
#include "LoggerRegistry.h"
#include <cassert>

namespace XYZEngine
{
	SpriteAnimationComponent::SpriteAnimationComponent(GameObject* gameObject) : Component(gameObject) {}

	void SpriteAnimationComponent::Update(float deltaTime)
	{
		if (renderer == nullptr)
		{
			renderer = gameObject->GetComponent<SpriteRendererComponent>();
			if (renderer == nullptr)
			{
				return;
			}

			renderer->SetVisible(isPlaying && delayTimer <= 0.f);
		}

		if (!isPlaying || frames.empty())
		{
			return;
		}

		if (delayTimer > 0.f)
		{
			delayTimer -= deltaTime;
			if (delayTimer > 0.f)
			{
				return;
			}

			ShowFirstFrame();
		}

		frameTimer += deltaTime;
		while (frameTimer >= secondsPerFrame)
		{
			frameTimer -= secondsPerFrame;
			currentFrame++;

			if (currentFrame < (int)frames.size())
			{
				continue;
			}

			if (isLooped)
			{
				currentFrame = 0;
				continue;
			}

			currentFrame = (int)frames.size() - 1;
			Finish();
			break;
		}

		renderer->SetTexture(*frames[currentFrame]);
	}
	void SpriteAnimationComponent::Render()
	{

	}

	void SpriteAnimationComponent::SetFrames(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
	{
		assert(framesCount > 0);
		assert(framesPerSecond > 0.f);

		frames.clear();

		int totalFrames = ResourceSystem::Instance()->GetTextureMapElementsCount(textureMapName);
		if (firstFrameIndex < 0 || framesCount <= 0 || firstFrameIndex + framesCount > totalFrames)
		{
			LOG_ERROR("Wrong animation frames range for texture map: " + textureMapName);
			return;
		}

		for (int i = firstFrameIndex; i < firstFrameIndex + framesCount; i++)
		{
			frames.push_back(ResourceSystem::Instance()->GetTextureMapElementShared(textureMapName, i));
		}

		secondsPerFrame = 1.f / framesPerSecond;
	}
	void SpriteAnimationComponent::SetLooped(bool newIsLooped)
	{
		isLooped = newIsLooped;
	}
	void SpriteAnimationComponent::SetEndBehaviour(SpriteAnimationEnd newEndBehaviour)
	{
		endBehaviour = newEndBehaviour;
	}
	void SpriteAnimationComponent::SetStartDelay(float newStartDelay)
	{
		startDelay = newStartDelay;
	}

	void SpriteAnimationComponent::Play()
	{
		if (frames.empty())
		{
			return;
		}

		isPlaying = true;
		delayTimer = startDelay;
		frameTimer = 0.f;
		currentFrame = 0;

		if (delayTimer <= 0.f)
		{
			ShowFirstFrame();
		}
	}
	void SpriteAnimationComponent::Stop()
	{
		isPlaying = false;
	}
	bool SpriteAnimationComponent::IsPlaying() const
	{
		return isPlaying;
	}

	void SpriteAnimationComponent::ShowFirstFrame()
	{
		if (renderer == nullptr)
		{
			return;
		}

		renderer->SetTexture(*frames[0]);
		renderer->SetVisible(true);
	}

	void SpriteAnimationComponent::Finish()
	{
		isPlaying = false;

		switch (endBehaviour)
		{
		case SpriteAnimationEnd::Hide:
			renderer->SetVisible(false);
			break;
		case SpriteAnimationEnd::Destroy:
			GameWorld::Instance()->DestroyGameObject(gameObject);
			break;
		default:
			break;
		}
	}
}
