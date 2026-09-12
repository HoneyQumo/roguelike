#include "pch.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "RenderSystem.h"
#include "TransformComponent.h"

namespace XYZEngine
{
	CameraComponent::CameraComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetTransform();
	}
	CameraComponent::~CameraComponent()
	{
		RenderSystem::Instance()->UnsubscribeResize(resizeSubscription);
	}

	void CameraComponent::Start()
	{
		resizeSubscription = RenderSystem::Instance()->SubscribeResize([this](unsigned int, unsigned int) { ApplyViewSize(); });
		ApplyViewSize();
	}
	void CameraComponent::Update(float deltaTime)
	{
		auto position = transform->GetWorldPosition();

		view.setCenter(Convert<sf::Vector2f, Vector2Df>(position));
		view.setRotation(isRotationEnabled ? transform->GetWorldRotation() : 0.f);

		RenderSystem::Instance()->GetMainWindow().setView(view);
	}
	void CameraComponent::Render()
	{
	}

	void CameraComponent::SetViewHeight(float newViewHeight)
	{
		if (newViewHeight <= 0.f)
		{
			LOG_WARN("View height must be greater than zero.");
			return;
		}

		viewHeight = newViewHeight;
		ApplyViewSize();
	}
	void CameraComponent::ApplyViewSize()
	{
		sf::Vector2u windowSize = RenderSystem::Instance()->GetWindowSize();
		float aspect = windowSize.y > 0 ? static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y) : 1.f;
		float height = viewHeight * zoom;

		view.setSize(height * aspect, -height);
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

		zoom *= newZoom;
		ApplyViewSize();
	}
}