#include "pch.h"
#include "UiScreen.h"

namespace XYZEngine
{
	UiWidget& UiScreen::GetRoot()
	{
		return root;
	}

	const UiWidget& UiScreen::GetRoot() const
	{
		return root;
	}

	void UiScreen::Resize(const sf::Vector2f& screenSize)
	{
		root.Layout({0.f, 0.f, screenSize.x, screenSize.y});
	}

	void UiScreen::Draw() const
	{
		root.Draw();
	}

	bool UiScreen::HandlePointer(const sf::Vector2f& point, bool isPressed, bool wasReleased)
	{
		return root.HandlePointer(point, isPressed, wasReleased);
	}

	void UiScreen::SetVisible(bool isVisible)
	{
		root.SetVisible(isVisible);
	}

	bool UiScreen::IsVisible() const
	{
		return root.IsVisible();
	}
}
