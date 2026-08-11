#pragma once

#include "ColliderComponent.h"
#include "TransformComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	class BoxColliderComponent : public ColliderComponent
	{
	public:
		BoxColliderComponent(GameObject* gameObject);
		~BoxColliderComponent();

		void Update(float deltaTime) override;
		void Render() override;

		void SetSize(float newWidth, float newHeight);
		Vector2Df GetSize() const;
	private:
		TransformComponent* transform;

		Vector2Df size = { 0.f, 0.f };
	};
}
