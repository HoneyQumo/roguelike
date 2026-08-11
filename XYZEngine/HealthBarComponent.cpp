#include "pch.h"
#include "HealthBarComponent.h"
#include "GameObject.h"
#include "RenderSystem.h"
#include "LoggerRegistry.h"

namespace XYZEngine
{
	HealthBarComponent::HealthBarComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();

		background = new sf::RectangleShape();
		background->setFillColor({ 20, 20, 20, 200 });

		fill = new sf::RectangleShape();
		fill->setFillColor({ 200, 60, 60 });
	}
	HealthBarComponent::~HealthBarComponent()
	{
		delete background;
		delete fill;
	}

	void HealthBarComponent::Update(float deltaTime)
	{
		if (health == nullptr)
		{
			health = gameObject->GetComponent<HealthComponent>();
			if (health == nullptr)
			{
				LOG_ERROR("HealthBar needs HealthComponent on " + gameObject->GetName());
				gameObject->RemoveComponent(this);
			}
		}
	}
	void HealthBarComponent::Render()
	{
		if (health == nullptr || !health->IsAlive() || health->GetHealthPercent() >= 1.f)
		{
			return;
		}

		auto position = transform->GetWorldPosition();
		sf::Vector2f barPosition = { position.x - 0.5f * size.x, position.y + offset.y };

		background->setSize({ size.x, size.y });
		background->setPosition(barPosition);

		fill->setSize({ size.x * health->GetHealthPercent(), size.y });
		fill->setPosition(barPosition);

		RenderSystem::Instance()->Render(*background);
		RenderSystem::Instance()->Render(*fill);
	}

	void HealthBarComponent::SetSize(float newWidth, float newHeight)
	{
		size = { newWidth, newHeight };
	}
	void HealthBarComponent::SetOffset(float offsetX, float offsetY)
	{
		offset = { offsetX, offsetY };
	}
	void HealthBarComponent::SetColors(const sf::Color& newFillColor, const sf::Color& newBackgroundColor)
	{
		fill->setFillColor(newFillColor);
		background->setFillColor(newBackgroundColor);
	}
}
