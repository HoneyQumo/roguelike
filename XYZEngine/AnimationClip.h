#pragma once

#include <string>
#include <vector>
#include <SFML/Graphics/Texture.hpp>

namespace XYZEngine
{
	class AnimationClip
	{
	public:
		bool Load(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
		void SetFrameSeconds(const float* newFrameSeconds);

		bool IsEmpty() const;
		int GetFramesCount() const;
		const sf::Texture* GetFrame(int frame) const;
		float GetFrameSeconds(int frame) const;

	private:
		std::vector<const sf::Texture*> frames;
		float secondsPerFrame = 0.1f;
		const float* frameSeconds = nullptr;

		int ClampFrame(int frame) const;
	};
}
