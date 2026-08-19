#pragma once

#include "Component.h"
#include "MovementComponent.h"
#include "SpriteRendererComponent.h"

namespace XYZEngine
{
	class SpriteDirectionComponent : public Component
	{
	public:
		SpriteDirectionComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;
	private:
		MovementComponent* movement = nullptr;
		SpriteRendererComponent* renderer = nullptr;
	};
}
