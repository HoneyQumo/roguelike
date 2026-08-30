#include "pch.h"
#include "GameWorld.h"

namespace XYZEngine
{
	GameWorld* GameWorld::Instance()
	{
		static GameWorld world;
		return &world;
	}

	void GameWorld::Update(float deltaTime)
	{
		for (int i = 0; i < gameObjects.size(); i++)
		{
			gameObjects[i]->Update(deltaTime);
		}
	}
	// Collisions are resolved once per frame: with a rarer step objects get drawn while still pushed into a wall.
	void GameWorld::FixedUpdate(float deltaTime)
	{
		PhysicsSystem::Instance()->Update();
	}
	void GameWorld::Render()
	{
		if (isRenderOrderDirty)
		{
			renderOrder = gameObjects;
			std::stable_sort(renderOrder.begin(), renderOrder.end(),
				[](const GameObject* first, const GameObject* second) { return first->GetRenderLayer() < second->GetRenderLayer(); });

			isRenderOrderDirty = false;
		}

		for (int i = 0; i < renderOrder.size(); i++)
		{
			renderOrder[i]->Render();
		}
	}
	void GameWorld::LateUpdate()
	{
		for (int i = markedToDestroyGameObjects.size() - 1; i >= 0; i--)
		{
			DestroyGameObjectImmediate(markedToDestroyGameObjects[i]);
		}
	}

	GameObject* GameWorld::CreateGameObject()
	{
		GameObject* newGameObject = new GameObject();
		gameObjects.push_back(newGameObject);
		isRenderOrderDirty = true;
		return newGameObject;
	}
	GameObject* GameWorld::CreateGameObject(std::string name)
	{
		GameObject* newGameObject = new GameObject(name);
		gameObjects.push_back(newGameObject);
		isRenderOrderDirty = true;
		return newGameObject;
	}
	GameObject* GameWorld::FindGameObject(const std::string& name) const
	{
		for (const auto& gameObject : gameObjects)
		{
			if (gameObject != nullptr && gameObject->GetName() == name)
			{
				return gameObject;
			}
		}

		return nullptr;
	}
	void GameWorld::DestroyGameObject(GameObject* gameObject)
	{
		markedToDestroyGameObjects.push_back(gameObject);
	}
	void GameWorld::InvalidateRenderOrder()
	{
		isRenderOrderDirty = true;
	}
	void GameWorld::Clear()
	{
		for (int i = gameObjects.size() - 1; i >= 0; i--)
		{
			if (gameObjects[i] == nullptr)
			{
				continue;
			}

			if (gameObjects[i]->GetComponent<TransformComponent>()->GetParent() == nullptr)
			{
				DestroyGameObjectImmediate(gameObjects[i]);
			}
		}
	}

	void GameWorld::Print() const
	{
		for (auto& obj : gameObjects)
		{
			if (obj == nullptr)
			{
				continue;
			}
			if (obj->GetComponent<TransformComponent>()->GetParent() == nullptr)
			{
				obj->Print();
			}
		}
	}

	void GameWorld::DestroyGameObjectImmediate(GameObject* gameObject)
	{
		auto parent = gameObject->GetComponent<TransformComponent>()->GetParent();
		if (parent != nullptr)
		{
			parent->GetGameObject()->RemoveChild(gameObject);
		}

		for (auto transform : gameObject->GetComponentsInChildren<TransformComponent>())
		{
			GameObject* gameObjectToDelete = transform->GetGameObject();

			gameObjects.erase(std::remove_if(gameObjects.begin(), gameObjects.end(), [gameObjectToDelete](GameObject* obj) { return obj == gameObjectToDelete; }), gameObjects.end());
			markedToDestroyGameObjects.erase(std::remove_if(markedToDestroyGameObjects.begin(), markedToDestroyGameObjects.end(), [gameObjectToDelete](GameObject* obj) { return obj == gameObjectToDelete; }), markedToDestroyGameObjects.end());

			delete gameObjectToDelete;
		}

		isRenderOrderDirty = true;
	}
}