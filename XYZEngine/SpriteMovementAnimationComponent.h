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

		void SetWalkAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
		void SetIdleAnimation(const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
	private:
		struct Animation
		{
			std::vector<const sf::Texture*> frames;
			float secondsPerFrame = 0.125f;
		};

		TransformComponent* transform;
		SpriteRendererComponent* renderer = nullptr;

		Animation walkAnimation;
		Animation idleAnimation;
		Animation* currentAnimation = nullptr;

		float frameTimer = 0.f;
		int currentFrame = 0;

		Vector2Df previousPosition = { 0.f, 0.f };
		float stillTimer = 0.f;

		void Fill(Animation& animation, const std::string& textureMapName, int firstFrameIndex, int framesCount, float framesPerSecond);
		void Play(Animation& animation);
	};
}
