#include "pch.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"

namespace XYZEngine
{
	CameraComponent::CameraComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();
	}
	void CameraComponent::Update(float deltaTime)
	{
		if (window == nullptr)
		{
			return;
		}

		auto position = transform->GetWorldPosition();

		view.setCenter(Convert<sf::Vector2f, Vector2Df>(position));
		view.setRotation(isRotationEnabled ? transform->GetWorldRotation() : 0.f);

		window->setView(view);
	}
	void CameraComponent::Render()
	{
		if (window == nullptr)
		{
			LOG_ERROR("Camera has no window on " + gameObject->GetName());
		}
	}

	void CameraComponent::SetBaseResolution(int width, int height)
	{
		view.reset(sf::FloatRect(0.f, 0.f, static_cast<float>(width), -static_cast<float>(height)));
	}
	void CameraComponent::SetWindow(sf::RenderWindow* newWindow)
	{
		window = newWindow;
	}
	void CameraComponent::SetRotationEnabled(bool newIsRotationEnabled)
	{
		isRotationEnabled = newIsRotationEnabled;
	}
	void CameraComponent::ZoomBy(float newZoom)
	{
		if (newZoom <= 0)
		{
			LOG_WARN("Zoom must be greater than zero.");
			return;
		}
		view.zoom(newZoom);
	}
}