#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "GameObject.h"
#include "PhysicsSystem.h"

namespace XYZEngine
{
	class GameWorld
	{
	public:
		static GameWorld* Instance();

		void Update(float deltaTime);
		void UpdatePhysics();
		void Render();
		void LateUpdate();

		GameObject* CreateGameObject();
		GameObject* CreateGameObject(std::string name);
		GameObject* CreateGameObject(std::string name, GameObject* parent);
		GameObject* FindGameObject(const std::string& name) const;

		// Жив ли ещё этот объект: единственный способ проверить сохранённый указатель.
		bool Contains(const GameObject* gameObject) const;

		template <typename T>
		T* FindComponent(const std::string& name) const
		{
			GameObject* found = FindGameObject(name);
			return found == nullptr ? nullptr : found->GetComponent<T>();
		}

		template <typename T>
		std::vector<T*> FindComponents() const
		{
			std::vector<T*> found;
			for (GameObject* gameObject : gameObjects)
			{
				if (T* component = gameObject->GetComponent<T>())
				{
					found.push_back(component);
				}
			}

			return found;
		}

		std::size_t GetObjectsCount() const;
		void DestroyGameObject(GameObject* gameObject);
		void DestroyGameObjects(const std::string& name);

		// Сносит всё, что помечено как принадлежащее локации.
		void DestroyTemporary();
		void Clear();

		void InvalidateRenderOrder();

		void Print() const;
	private:
		GameWorld() {}
		~GameWorld() {}

		GameWorld(GameWorld const&) = delete;
		GameWorld& operator= (GameWorld const&) = delete;

		GameObjectId nextId = NO_GAME_OBJECT + 1;
		std::vector<GameObject*> gameObjects = {};
		std::unordered_map<std::string, std::vector<GameObject*>> gameObjectsByName;
		std::vector<GameObject*> markedToDestroyGameObjects = {};
		std::vector<GameObject*> renderOrder = {};
		bool isRenderOrderDirty = true;

		void DestroyGameObjectImmediate(GameObject* gameObject);
		void RegisterGameObject(GameObject* gameObject);
		void UnregisterGameObject(GameObject* gameObject);
	};
}