#pragma once

#include "TransformComponent.h"
#include "LoggerRegistry.h"
#include <iostream>

namespace XYZEngine
{
	class TransformComponent;

	class GameObject
	{
	public:
		GameObject();
		GameObject(std::string newName);

		~GameObject();

		std::string GetName() const;
		void Print(int depth = 0) const;

		void SetRenderLayer(int newRenderLayer);
		int GetRenderLayer() const;

		void Update(float deltaTime);
		void Render();

		template <typename T>
		T* AddComponent()
		{
			if constexpr (!std::is_base_of<Component, T>::value)
			{
				LOG_ERROR("T must be derived from Component.");
				return nullptr;
			}

			if constexpr (std::is_same<T, TransformComponent>::value)
			{
				if (GetComponent<TransformComponent>() != nullptr)
				{
					LOG_WARN("Can't add second Transform to " + name);
					return nullptr;
				}
			}

			T* newComponent = new T(this);
			components.push_back(newComponent);

			return newComponent;
		}

		void RemoveComponent(Component* component)
		{
			components.erase(std::remove_if(components.begin(), components.end(), [component](Component* obj) { return obj == component; }), components.end());
			delete component;
		}

		template <typename T>
		T* GetComponent() const
		{
			for (const auto& component : components)
			{
				if (auto casted = dynamic_cast<T*>(component))
				{
					return casted;
				}
			}
			return nullptr;
		}

		template <typename T>
		T* GetComponentInChildren() const
		{
			T* component = GetComponent<T>();
			if (component != nullptr || children.size() == 0)
			{
				return component;
			}

			for (const auto& child : children)
			{
				T* childComponent = child->GetComponentInChildren<T>();
				if (childComponent != nullptr)
				{
					return childComponent;
				}
			}

			return nullptr;
		}

		template <typename T>
		std::vector<T*> GetComponents() const
		{
			std::vector<T*> result;
			for (const auto& component : components)
			{
				if (auto casted = dynamic_cast<T*>(component))
				{
					result.push_back(casted);
				}
			}
			return result;
		}

		template <typename T>
		std::vector<T*> GetComponentsInChildren() const
		{
			std::vector<T*> result;
			for (const auto& component : GetComponents<T>())
			{
				result.push_back(component);
			}

			for (const auto& child : children)
			{
				for (const auto& childComponent : child->GetComponentsInChildren<T>())
				{
					result.push_back(childComponent);
				}
			}

			return result;
		}

		friend class GameWorld;
		friend class TransformComponent;

	private:
		std::string name;
		int renderLayer = 0;

		std::vector<GameObject*> children = {};
		std::vector<Component*> components = {};

		void AddChild(GameObject* child);
		void RemoveChild(GameObject* child);
	};
}