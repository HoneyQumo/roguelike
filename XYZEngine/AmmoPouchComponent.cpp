#include "pch.h"
#include "AmmoPouchComponent.h"
#include "GameObject.h"
#include "LoggerRegistry.h"
#include <algorithm>

namespace XYZEngine
{
	AmmoPouchComponent::AmmoPouchComponent(GameObject* gameObject) : Component(gameObject) {}

	void AmmoPouchComponent::Update(float deltaTime)
	{
	}
	void AmmoPouchComponent::Render()
	{
	}

	void AmmoPouchComponent::SetAmmo(int ammoKind, int count)
	{
		if (count < 0)
		{
			LOG_WARN("Ammo count can't be negative on " + gameObject->GetName());
			count = 0;
		}

		reserve[ammoKind] = count;
	}

	void AmmoPouchComponent::AddAmmo(int ammoKind, int count)
	{
		if (count <= 0)
		{
			return;
		}

		reserve[ammoKind] += count;
	}

	int AmmoPouchComponent::GetAmmo(int ammoKind) const
	{
		auto ammoPair = reserve.find(ammoKind);
		return ammoPair == reserve.end() ? 0 : ammoPair->second;
	}

	int AmmoPouchComponent::TakeAmmo(int ammoKind, int count)
	{
		if (count <= 0)
		{
			return 0;
		}

		auto ammoPair = reserve.find(ammoKind);
		if (ammoPair == reserve.end())
		{
			return 0;
		}

		int taken = std::min(count, ammoPair->second);
		ammoPair->second -= taken;

		return taken;
	}
}
