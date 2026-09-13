#pragma once

#include "ParticleSpec.h"
#include "Vector.h"

namespace XYZEngine
{
	class ParticleSystemComponent;

	class ParticleSystem
	{
	public:
		static ParticleSystem* Instance();

		void Register(ParticleSystemComponent* component);
		void Unregister(ParticleSystemComponent* component);
		ParticleSystemComponent* GetActive() const;

		void Emit(const ParticleSpec& spec, const Vector2Df& position, const Vector2Df& direction);
		void Clear();
		void EmitCount(const ParticleSpec& spec, const Vector2Df& position, const Vector2Df& direction, int count);

	private:
		ParticleSystemComponent* active = nullptr;

		ParticleSystem() {}
		~ParticleSystem() {}

		ParticleSystem(ParticleSystem const&) = delete;
		ParticleSystem& operator=(ParticleSystem const&) = delete;
	};
}
