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

		template <typename T>
		T* FindComponent(const std::string& name) const
		{
			GameObject* found = FindGameObject(name);
			return found == nullptr ? nullptr : found->GetComponent<T>();
		}

		std::size_t GetObjectsCount() const;
		void DestroyGameObject(GameObject* gameObject);
		void DestroyGameObjects(const std::string& name);
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