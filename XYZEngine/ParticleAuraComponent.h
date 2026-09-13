#pragma once

#include "Component.h"
#include "ParticleSpec.h"
#include "TransformComponent.h"

namespace XYZEngine
{
	class ParticleAuraComponent : public Component
	{
	public:
		ParticleAuraComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		void SetSpec(const ParticleSpec* newSpec);
		void SetRadius(float newRadius);
		void SetActive(bool newIsActive);

		bool IsActive() const;
		float GetRadius() const;

	private:
		TransformComponent* transform = nullptr;
		const ParticleSpec* spec = nullptr;

		float radius = 0.f;
		float angle = 0.f;
		float pending = 0.f;
		bool isActive = false;
	};
}
