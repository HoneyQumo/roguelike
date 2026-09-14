#include "pch.h"
#include "UiPanel.h"
#include "RenderSystem.h"

namespace XYZEngine
{
	void UiPanel::SetFillColor(const sf::Color& color)
	{
		shape.setFillColor(color);
	}

	void UiPanel::SetOutline(float thickness, const sf::Color& color)
	{
		shape.setOutlineThickness(thickness);
		shape.setOutlineColor(color);
	}

	const sf::RectangleShape& UiPanel::GetShape() const
	{
		return shape;
	}

	void UiPanel::OnLayout()
	{
		const sf::FloatRect& bounds = GetBounds();
		shape.setPosition(bounds.left, bounds.top);
		shape.setSize({bounds.width, bounds.height});
	}

	void UiPanel::OnDraw() const
	{
		RenderSystem::Instance()->Render(shape);
	}
}
