#include "pch.h"
#include "SpriteAnimationComponent.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "LoggerRegistry.h"
#include <cassert>

namespace XYZEngine
{
	SpriteAnimationComponent::SpriteAnimationComponent(GameObject* gameObject) : Component(gameObject) {}

	void SpriteAnimationComponent::Start()
	{
		renderer = gameObject->GetComponent<SpriteRendererComponent>();
		if (renderer == nullptr)
		{
			return;
		}

		renderer->SetVisible(isPlaying && startDelay.IsReady());
	}

	void SpriteAnimationComponent::Update(float deltaTime)
	{
		if (renderer == nullptr)
		{
			return;
		}

		if (!isPlaying || clip.IsEmpty())
		{
			return;
		}

		if (startDelay.IsRunning())
		{
			startDelay.Tick(deltaTime);
			if (startDelay.IsRunning())
			{
				return;
			}

			ShowFirstFrame();
		}

		frameTimer += deltaTime;
		while (frameTimer >= clip.GetFrameSeconds(currentFrame))
		{
			frameTimer -= clip.GetFrameSeconds(currentFrame);
			currentFrame++;

			if (currentFrame < clip.GetFramesCount())
			{
				continue;
			}

			if (isLooped)
			{
				currentFrame = 0;
				continue;
			}

			currentFrame = clip.GetFramesCount() - 1;
			Finish();
			break;
		}

		renderer->SetTexture(*clip.GetFrame(currentFrame));
	}
	void SpriteAnimationComponent::Render()
	{

	}

	void SpriteAnimationComponent::SetFrames(const std::string& textureMapName, int firstFrameIndex, int framesCount, float secondsPerFrame)
	{
		assert(framesCount > 0);
		assert(secondsPerFrame > 0.f);

		clip.Load(textureMapName, firstFrameIndex, framesCount, secondsPerFrame);
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
		startDelay.SetDuration(newStartDelay);
	}

	void SpriteAnimationComponent::Play()
	{
		if (clip.IsEmpty())
		{
			return;
		}

		isPlaying = true;
		startDelay.Restart();
		frameTimer = 0.f;
		currentFrame = 0;

		if (startDelay.IsReady())
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

		renderer->SetTexture(*clip.GetFrame(0));
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
