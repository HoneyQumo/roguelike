#include "pch.h"
#include "Component.h"

namespace XYZEngine
{
	Component::Component(GameObject* gameObject) : gameObject(gameObject) {}
	Component::~Component()
	{
	}
	GameObject* Component::GetGameObject() const
	{
		return gameObject;
	}
	void Component::SetEnabled(bool newIsEnabled)
	{
		if (isEnabled == newIsEnabled)
		{
			return;
		}

		isEnabled = newIsEnabled;

		if (isEnabled)
		{
			OnEnable();
		}
		else
		{
			OnDisable();
		}
	}
	bool Component::IsEnabled() const
	{
		return isEnabled;
	}
	bool Component::IsDestroyed() const
	{
		return isDestroyed;
	}
}