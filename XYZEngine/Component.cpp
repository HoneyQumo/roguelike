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
	bool Component::IsDestroyed() const
	{
		return isDestroyed;
	}
}