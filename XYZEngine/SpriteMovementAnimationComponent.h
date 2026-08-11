#pragma once

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Component.h"
#include "TransformComponent.h"
#include "SpriteRendererComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	class SpriteMovementAnimationComponent : public Component
	{
	public:
		SpriteMovementAnimationComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		void SetFrames(const std::string& textureMapName, int firstFrameIndex, int framesCount);
		void SetFramerate(float newFramesPerSecond);
	private:
		TransformComponent* transform;
		SpriteRendererComponent* renderer = nullptr;

		std::vector<const sf::Texture*> frames;
		float secondsPerFrame = 0.125f;
		float frameTimer = 0.f;
		int currentFrame = 0;

		Vector2Df previousPosition = { 0.f, 0.f };

		void ShowFrame(int frameIndex);
	};
}
