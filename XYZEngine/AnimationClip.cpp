#include "pch.h"
#include "AnimationClip.h"
#include "ResourceSystem.h"
#include "LoggerRegistry.h"
#include <algorithm>

namespace XYZEngine
{
	bool AnimationClip::Load(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond)
	{
		frames.clear();

		if (framesPerSecond <= 0.f)
		{
			LOG_ERROR("Framerate must be positive for texture map: " + textureMapName);
			return false;
		}

		int totalFrames = ResourceSystem::Instance()->GetTextureMapElementsCount(textureMapName);
		if (firstFrameIndex < 0 || framesCount <= 0 || firstFrameIndex + framesCount > totalFrames)
		{
			LOG_ERROR("Wrong animation frames range for texture map: " + textureMapName);
			return false;
		}

		frames.reserve(framesCount);
		for (int i = firstFrameIndex; i < firstFrameIndex + framesCount; i++)
		{
			frames.push_back(ResourceSystem::Instance()->GetTextureMapElementShared(textureMapName, i));
		}

		secondsPerFrame = 1.f / framesPerSecond;
		return true;
	}

	// Покадровые длительности задаются таблицей игры и живут дольше клипа.
	void AnimationClip::SetFrameSeconds(const float* newFrameSeconds)
	{
		frameSeconds = newFrameSeconds;
	}

	bool AnimationClip::IsEmpty() const
	{
		return frames.empty();
	}

	int AnimationClip::GetFramesCount() const
	{
		return static_cast<int>(frames.size());
	}

	const sf::Texture* AnimationClip::GetFrame(int frame) const
	{
		if (frames.empty())
		{
			return nullptr;
		}

		return frames[ClampFrame(frame)];
	}

	float AnimationClip::GetFrameSeconds(int frame) const
	{
		if (frameSeconds == nullptr || frames.empty())
		{
			return secondsPerFrame;
		}

		return frameSeconds[ClampFrame(frame)];
	}

	int AnimationClip::ClampFrame(int frame) const
	{
		return std::min(std::max(frame, 0), GetFramesCount() - 1);
	}
}
