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
	void GameWorld::UpdatePhysics()
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
		while (!markedToDestroyGameObjects.empty())
		{
			GameObject* gameObject = markedToDestroyGameObjects.back();
			markedToDestroyGameObjects.pop_back();

			DestroyGameObjectImmediate(gameObject);
		}

		for (int i = 0; i < gameObjects.size(); i++)
		{
			gameObjects[i]->DestroyMarkedComponents();
		}
	}

	GameObject* GameWorld::CreateGameObject()
	{
		GameObject* newGameObject = new GameObject();
		RegisterGameObject(newGameObject);
		return newGameObject;
	}
	GameObject* GameWorld::CreateGameObject(std::string name)
	{
		GameObject* newGameObject = new GameObject(name);
		RegisterGameObject(newGameObject);
		return newGameObject;
	}
	// Дочерний объект начинает в начале координат родителя: SetParent сохраняет мировую позицию.
	GameObject* GameWorld::CreateGameObject(std::string name, GameObject* parent)
	{
		GameObject* child = CreateGameObject(std::move(name));
		if (parent == nullptr)
		{
			LOG_WARN("Child object " + child->GetName() + " has no parent");
			return child;
		}

		auto childTransform = child->GetTransform();
		childTransform->SetParent(parent->GetTransform());
		childTransform->SetLocalPosition(0.f, 0.f);

		return child;
	}
	GameObject* GameWorld::FindGameObject(const std::string& name) const
	{
		auto found = gameObjectsByName.find(name);
		if (found == gameObjectsByName.end() || found->second.empty())
		{
			return nullptr;
		}

		return found->second.front();
	}
	std::size_t GameWorld::GetObjectsCount() const
	{
		return gameObjects.size();
	}
	/**
	*	Объект, которого в мире уже нет, молча пропускается.
	*
	*	Указатели на GameObject разданы половине игры и никак не помечаются
	*	при удалении, поэтому повторное удаление раньше разыменовывало освобождённую память.
	*	Значение указателя сравнивать безопасно - разыменовывать нет.
	*/
	bool GameWorld::Contains(const GameObject* gameObject) const
	{
		return gameObject != nullptr && std::find(gameObjects.begin(), gameObjects.end(), gameObject) != gameObjects.end();
	}

	void GameWorld::DestroyGameObject(GameObject* gameObject)
	{
		if (gameObject == nullptr || std::find(gameObjects.begin(), gameObjects.end(), gameObject) == gameObjects.end())
		{
			return;
		}

		markedToDestroyGameObjects.push_back(gameObject);
	}
	void GameWorld::DestroyGameObjects(const std::string& name)
	{
		auto found = gameObjectsByName.find(name);
		if (found == gameObjectsByName.end())
		{
			return;
		}

		for (GameObject* gameObject : found->second)
		{
			DestroyGameObject(gameObject);
		}
	}
	void GameWorld::InvalidateRenderOrder()
	{
		isRenderOrderDirty = true;
	}
	void GameWorld::Clear()
	{
		while (!gameObjects.empty())
		{
			GameObject* toDestroy = gameObjects.front();
			for (GameObject* gameObject : gameObjects)
			{
				if (gameObject->GetTransform()->GetParent() == nullptr)
				{
					toDestroy = gameObject;
					break;
				}
			}

			DestroyGameObjectImmediate(toDestroy);
		}

		markedToDestroyGameObjects.clear();
	}

	void GameWorld::Print() const
	{
		for (auto& obj : gameObjects)
		{
			if (obj == nullptr)
			{
				continue;
			}
			if (obj->GetTransform()->GetParent() == nullptr)
			{
				obj->Print();
			}
		}
	}

	void GameWorld::DestroyGameObjectImmediate(GameObject* gameObject)
	{
		auto parent = gameObject->GetTransform()->GetParent();
		if (parent != nullptr)
		{
			parent->GetGameObject()->RemoveChild(gameObject);
		}

		for (auto transform : gameObject->GetComponentsInChildren<TransformComponent>())
		{
			GameObject* gameObjectToDelete = transform->GetGameObject();

			UnregisterGameObject(gameObjectToDelete);
			markedToDestroyGameObjects.erase(std::remove_if(markedToDestroyGameObjects.begin(), markedToDestroyGameObjects.end(), [gameObjectToDelete](GameObject* obj) { return obj == gameObjectToDelete; }), markedToDestroyGameObjects.end());

			delete gameObjectToDelete;
		}

		isRenderOrderDirty = true;
	}

	void GameWorld::RegisterGameObject(GameObject* gameObject)
	{
		gameObject->id = nextId++;
		gameObjects.push_back(gameObject);
		gameObjectsByName[gameObject->GetName()].push_back(gameObject);
		isRenderOrderDirty = true;
	}

	void GameWorld::UnregisterGameObject(GameObject* gameObject)
	{
		gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), gameObject), gameObjects.end());

		auto named = gameObjectsByName.find(gameObject->GetName());
		if (named != gameObjectsByName.end())
		{
			auto& sameName = named->second;
			sameName.erase(std::remove(sameName.begin(), sameName.end(), gameObject), sameName.end());
			if (sameName.empty())
			{
				gameObjectsByName.erase(named);
			}
		}
	}
}