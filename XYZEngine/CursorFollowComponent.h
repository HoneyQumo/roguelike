#pragma once

#include "Component.h"
#include "TransformComponent.h"

namespace XYZEngine
{
	class CursorFollowComponent : public Component
	{
	public:
		CursorFollowComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;
	private:
		TransformComponent* transform;
	};
}
