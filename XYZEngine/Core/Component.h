#pragma once

namespace XYZEngine
{
	class GameObject;

	class Component
	{
	public:
		Component(GameObject* gameObject);
		virtual ~Component();

		virtual void Start() {}
		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;

		GameObject* GetGameObject() const;

		void SetEnabled(bool newIsEnabled);
		bool IsEnabled() const;

		bool IsDestroyed() const;

		friend class GameObject;
	protected:
		virtual void OnEnable() {}
		virtual void OnDisable() {}

		GameObject* gameObject = nullptr;
	private:
		bool isStarted = false;
		bool isDestroyed = false;
		bool isEnabled = true;
	};
}