#include "pch.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "MathUtils.h"
#include "RenderSystem.h"
#include "TransformComponent.h"
#include <algorithm>
#include <cmath>

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
		UpdateShake(deltaTime);

		auto position = transform->GetWorldPosition() + shakeOffset;

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
	void CameraComponent::Shake(const CameraShake& newShake)
	{
		if (newShake.amplitude <= 0.f || newShake.duration <= 0.f)
		{
			return;
		}

		float leftAmplitude = shake.amplitude * GetShakeDecay();
		float leftDuration = shake.duration - shakeTime;

		shake.amplitude = std::min(std::max(newShake.amplitude, leftAmplitude), maxShakeAmplitude);
		shake.duration = std::max(newShake.duration, leftDuration);
		shake.frequency = newShake.frequency;
		shakeTime = 0.f;
	}
	void CameraComponent::StopShake()
	{
		shake = CameraShake();
		shakeTime = 0.f;
		shakeOffset = {0.f, 0.f};
	}
	const Vector2Df& CameraComponent::GetShakeOffset() const
	{
		return shakeOffset;
	}
	void CameraComponent::SetMaxShakeAmplitude(float newMaxShakeAmplitude)
	{
		maxShakeAmplitude = std::max(newMaxShakeAmplitude, 0.f);
	}

	void CameraComponent::UpdateShake(float deltaTime)
	{
		if (shake.duration <= 0.f)
		{
			shakeOffset = {0.f, 0.f};
			return;
		}

		shakeTime += deltaTime;
		if (shakeTime >= shake.duration)
		{
			StopShake();
			return;
		}

		float decay = GetShakeDecay();
		float angle = TWO_PI * shake.frequency * shakeTime;

		shakeOffset.x = shake.amplitude * decay * std::sin(angle);
		shakeOffset.y = shake.amplitude * decay * std::sin(angle * 1.37f + 1.1f);
	}
	float CameraComponent::GetShakeDecay() const
	{
		if (shake.duration <= 0.f)
		{
			return 0.f;
		}

		float left = 1.f - shakeTime / shake.duration;
		return left <= 0.f ? 0.f : left * left;
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