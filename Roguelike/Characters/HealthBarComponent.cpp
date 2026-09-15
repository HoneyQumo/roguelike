#include "HealthBarComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <RenderSystem.h>
#include <LoggerRegistry.h>

using namespace XYZEngine;

namespace RoguelikeGame
{
	HealthBarComponent::HealthBarComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetTransform();

		background.setFillColor({ 20, 20, 20, 200 });
		fill.setFillColor({ 200, 60, 60 });
		armorFill.setFillColor(ARMOR_BAR_COLOR);
	}

	void HealthBarComponent::Start()
	{
		health = gameObject->GetComponent<HealthComponent>();
		if (health == nullptr)
		{
			LOG_ERROR("HealthBar needs HealthComponent on " + gameObject->GetName());
			gameObject->DestroyComponent(this);
		}
	}

	void HealthBarComponent::Update(float deltaTime)
	{
	}
	void HealthBarComponent::Render()
	{
		bool isUntouched = health != nullptr && health->GetHealthPercent() >= 1.f && health->GetArmorPercent() >= 1.f;
		if (health == nullptr || !health->IsAlive() || (!isAlwaysVisible && isUntouched))
		{
			return;
		}

		auto position = transform->GetWorldPosition();
		sf::Vector2f barPosition = { position.x - 0.5f * size.x, position.y + offset.y };

		background.setSize({ size.x, size.y });
		background.setPosition(barPosition);

		fill.setSize({ size.x * health->GetHealthPercent(), size.y });
		fill.setPosition(barPosition);

		RenderSystem::Instance()->Render(background);
		RenderSystem::Instance()->Render(fill);

		if (health->GetArmor() > 0.f)
		{
			armorFill.setSize({ size.x * health->GetArmorPercent(), ARMOR_BAR_HEIGHT });
			armorFill.setPosition({ barPosition.x, barPosition.y - ARMOR_BAR_HEIGHT - ARMOR_BAR_GAP });

			RenderSystem::Instance()->Render(armorFill);
		}
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
		fill.setFillColor(newFillColor);
		background.setFillColor(newBackgroundColor);
	}

	void HealthBarComponent::SetAlwaysVisible(bool newIsAlwaysVisible)
	{
		isAlwaysVisible = newIsAlwaysVisible;
	}
}
