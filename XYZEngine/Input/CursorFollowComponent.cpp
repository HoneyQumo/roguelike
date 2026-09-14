#include "pch.h"
#include "CursorFollowComponent.h"
#include "GameObject.h"
#include "InputSystem.h"
#include "RenderSystem.h"
#include "Vector.h"

namespace XYZEngine
{
	CursorFollowComponent::CursorFollowComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetTransform();
	}

	void CursorFollowComponent::Update(float deltaTime)
	{
		auto& window = RenderSystem::Instance()->GetMainWindow();
		auto worldPosition = window.mapPixelToCoords(InputSystem::Instance()->GetMousePosition());

		transform->SetWorldPosition(Convert<Vector2Df, sf::Vector2f>(worldPosition));
	}
	void CursorFollowComponent::Render()
	{

	}
}
