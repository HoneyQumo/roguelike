#pragma once

namespace XYZEngine
{
	class GameObject;

	class Component
	{
	public:
		Component(GameObject* gameObject);
		virtual ~Component();

		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;

		GameObject* GetGameObject();

		bool IsDestroyed() const;

		friend class GameObject;
	protected:
		GameObject* gameObject;
	private:
		bool isDestroyed = false;
	};
}