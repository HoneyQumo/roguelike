#pragma once

#include "GameObject.h"
#include "PhysicsSystem.h"

namespace XYZEngine
{
	class GameWorld
	{
	public:
		static GameWorld* Instance();

		void Update(float deltaTime);
		void FixedUpdate(float deltaTime);
		void Render();
		void LateUpdate();

		GameObject* CreateGameObject();
		GameObject* CreateGameObject(std::string name);
		GameObject* FindGameObject(const std::string& name) const;
		void DestroyGameObject(GameObject* gameObject);
		void Clear();

		void InvalidateRenderOrder();

		void Print() const;
	private:
		GameWorld() {}
		~GameWorld() {}

		GameWorld(GameWorld const&) = delete;
		GameWorld& operator= (GameWorld const&) = delete;

		std::vector<GameObject*> gameObjects = {};
		std::vector<GameObject*> markedToDestroyGameObjects = {};
		std::vector<GameObject*> renderOrder = {};
		bool isRenderOrderDirty = true;

		void DestroyGameObjectImmediate(GameObject* gameObject);
	};
}