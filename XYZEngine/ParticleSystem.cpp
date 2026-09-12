#include "pch.h"
#include "ParticleSystem.h"
#include "LoggerRegistry.h"
#include "ParticleSystemComponent.h"

namespace XYZEngine
{
	ParticleSystem* ParticleSystem::Instance()
	{
		static ParticleSystem particles;
		return &particles;
	}

	void ParticleSystem::Register(ParticleSystemComponent* component)
	{
		if (component == nullptr)
		{
			return;
		}

		if (active != nullptr && active != component)
		{
			LOG_WARN("Second particle system replaces the active one");
		}

		active = component;
	}

	void ParticleSystem::Unregister(ParticleSystemComponent* component)
	{
		if (active == component)
		{
			active = nullptr;
		}
	}

	ParticleSystemComponent* ParticleSystem::GetActive() const
	{
		return active;
	}

	void ParticleSystem::Emit(const ParticleSpec& spec, const Vector2Df& position, const Vector2Df& direction)
	{
		if (active == nullptr)
		{
			return;
		}

		active->Emit(spec, position, direction);
	}

	void ParticleSystem::EmitCount(const ParticleSpec& spec, const Vector2Df& position, const Vector2Df& direction, int count)
	{
		if (active == nullptr)
		{
			return;
		}

		active->EmitCount(spec, position, direction, count);
	}
}
