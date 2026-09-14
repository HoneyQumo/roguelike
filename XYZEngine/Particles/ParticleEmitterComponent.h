#pragma once

#include "Component.h"
#include "ParticleSpec.h"
#include "TransformComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	class ParticleEmitterComponent : public Component
	{
	public:
		ParticleEmitterComponent(GameObject* gameObject);

		void Start() override;
		void Update(float deltaTime) override;
		void Render() override;

		void SetSpec(const ParticleSpec* newSpec);
		void SetDirection(const Vector2Df& newDirection);

		bool IsEmitting() const;

	private:
		TransformComponent* transform = nullptr;
		const ParticleSpec* spec = nullptr;

		Vector2Df direction = {0.f, 1.f};
		float pending = 0.f;
		float timeLeft = 0.f;
		bool isEmitting = false;

		void EmitPending();
	};
}
