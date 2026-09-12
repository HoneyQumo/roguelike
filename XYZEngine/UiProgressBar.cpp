#include "pch.h"
#include "UiProgressBar.h"
#include "RenderSystem.h"
#include <algorithm>

namespace XYZEngine
{
	void UiProgressBar::SetValue(float newValue)
	{
		value = std::clamp(newValue, 0.f, 1.f);
		ApplyFill();
	}

	float UiProgressBar::GetValue() const
	{
		return value;
	}

	void UiProgressBar::SetColors(const sf::Color& fill, const sf::Color& background)
	{
		fillShape.setFillColor(fill);
		backgroundShape.setFillColor(background);
	}

	sf::FloatRect UiProgressBar::GetFillBounds() const
	{
		return fillShape.getGlobalBounds();
	}

	void UiProgressBar::OnLayout()
	{
		const sf::FloatRect& bounds = GetBounds();

		backgroundShape.setPosition(bounds.left, bounds.top);
		backgroundShape.setSize({bounds.width, bounds.height});

		ApplyFill();
	}

	void UiProgressBar::ApplyFill()
	{
		const sf::FloatRect& bounds = GetBounds();

		fillShape.setPosition(bounds.left, bounds.top);
		fillShape.setSize({bounds.width * value, bounds.height});
	}

	void UiProgressBar::OnDraw() const
	{
		RenderSystem::Instance()->Render(backgroundShape);
		RenderSystem::Instance()->Render(fillShape);
	}
}
