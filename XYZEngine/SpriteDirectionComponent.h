#pragma once

#include "Component.h"
#include "TransformComponent.h"
#include "SpriteRendererComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	class SpriteDirectionComponent : public Component
	{
	public:
		SpriteDirectionComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;
	private:
		TransformComponent* transform;
		SpriteRendererComponent* renderer = nullptr;

		Vector2Df previousPosition = { 0.f, 0.f };
	};
}
