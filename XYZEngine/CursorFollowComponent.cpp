#include "pch.h"
#include "CursorFollowComponent.h"
#include "GameObject.h"
#include "RenderSystem.h"
#include "Vector.h"
#include <SFML/Window/Mouse.hpp>

namespace XYZEngine
{
	CursorFollowComponent::CursorFollowComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();
	}

	void CursorFollowComponent::Update(float deltaTime)
	{
		auto& window = RenderSystem::Instance()->GetMainWindow();
		auto worldPosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

		transform->SetWorldPosition(Convert<Vector2Df, sf::Vector2f>(worldPosition));
	}
	void CursorFollowComponent::Render()
	{

	}
}
