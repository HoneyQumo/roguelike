#include "pch.h"
#include "GameObject.h"
#include "GameWorld.h"

namespace XYZEngine
{
	GameObject::GameObject()
	{
		name = "GameObject";
		AddComponent<TransformComponent>();
	}
	GameObject::GameObject(std::string newName)
	{
		name = newName;
		AddComponent<TransformComponent>();
	}

	GameObject::~GameObject()
	{
		for (auto component : components)
		{
			delete component;
		}
		components.clear();
		markedToDestroyComponents.clear();
		children.clear();
	}

	void GameObject::DestroyComponent(Component* component)
	{
		if (component == nullptr || component->isDestroyed)
		{
			return;
		}

		component->isDestroyed = true;
		markedToDestroyComponents.push_back(component);
	}

	void GameObject::DestroyMarkedComponents()
	{
		if (markedToDestroyComponents.empty())
		{
			return;
		}

		std::vector<Component*> destroying;
		destroying.swap(markedToDestroyComponents);

		for (auto component : destroying)
		{
			components.erase(std::remove(components.begin(), components.end(), component), components.end());
			delete component;
		}
	}

	const std::string& GameObject::GetName() const
	{
		return name;
	}

	void GameObject::Print(int depth) const
	{
		std::cout << std::string(depth * 2, ' ') << GetName() << std::endl;
		for (auto& component : components)
		{
			std::cout << std::string(depth * 2, ' ') << "::" << component << std::endl;
		}

		for (GameObject* child : children)
		{
			child->Print(depth + 1);
		}
	}

	void GameObject::Update(float deltaTime)
	{
		for (int i = 0; i < components.size(); i++)
		{
			if (components[i]->isDestroyed || components[i]->isStarted)
			{
				continue;
			}

			components[i]->isStarted = true;
			components[i]->Start();
		}

		for (int i = 0; i < components.size(); i++)
		{
			if (!components[i]->isDestroyed)
			{
				components[i]->Update(deltaTime);
			}
		}
	}
	void GameObject::Render()
	{
		for (int i = 0; i < components.size(); i++)
		{
			if (!components[i]->isDestroyed)
			{
				components[i]->Render();
			}
		}
	}

	void GameObject::SetRenderLayer(int newRenderLayer)
	{
		renderLayer = newRenderLayer;

		for (GameObject* child : children)
		{
			child->SetRenderLayer(newRenderLayer);
		}

		GameWorld::Instance()->InvalidateRenderOrder();
	}
	int GameObject::GetRenderLayer() const
	{
		return renderLayer;
	}

	void GameObject::AddChild(GameObject* child)
	{
		children.push_back(child);
	}
	void GameObject::RemoveChild(GameObject* child)
	{
		children.erase(std::remove_if(children.begin(), children.end(), [child](GameObject* obj) { return obj == child; }), children.end());
	}
}